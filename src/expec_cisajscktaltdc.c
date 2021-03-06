/* HPhi  -  Quantum Lattice Model Simulator */
/* Copyright (C) 2015 The University of Tokyo */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "mltply.h"
#include "FileIO.h"
#include "bitcalc.h"
#include "expec_cisajscktaltdc.h"
#include "wrapperMPI.h"
#include "mltplyMPI.h"

/**
 * @file   expec_cisajscktaltdc.c
 * 
 * @brief  File for calculating two-body green's functions
 * 
 * @version 0.2
 * @details add function to treat the case of general spin
 *
 * @version 0.1
 * @author Takahiro Misawa (The University of Tokyo)
 * @author Kazuyoshi Yoshimi (The University of Tokyo)
 * 
 */

/** 
 * @brief Parent function to calculate two-body green's functions
 * 
 * @param X data list for calculation
 * @param vec eigenvectors
 * 
 * @retval 0 normally finished
 * @retval -1 unnormally finished
 * @note the origin of function's name cisajscktalt comes from c=creation, i=ith site, s=spin, a=annihiration, j=jth site and so on.
 *
 * @version 0.2
 * @details add function to treat the case of general spin
 *
 * @version 0.1
 * @author Takahiro Misawa (The University of Tokyo)
 * @author Kazuyoshi Yoshimi (The University of Tokyo)
 */
int expec_cisajscktaltdc
(
 struct BindStruct *X,
 double complex *vec
 )
{

  FILE *fp;
  char sdt[D_FileNameMax];

  long unsigned int i,j;
  long unsigned int irght,ilft,ihfbit;
  long unsigned int isite1,isite2,isite3,isite4;
  long unsigned int org_isite1,org_isite2,org_isite3,org_isite4;
  long unsigned int org_sigma1,org_sigma2,org_sigma3,org_sigma4;
  long unsigned int tmp_org_isite1,tmp_org_isite2,tmp_org_isite3,tmp_org_isite4;
  long unsigned int tmp_org_sigma1,tmp_org_sigma2,tmp_org_sigma3,tmp_org_sigma4;
  long unsigned int isA_up, isB_up;
  long unsigned int is1_up, is2_up;
  long unsigned int Asum,Bsum,Adiff,Bdiff;
  long unsigned int tmp_off=0;
  long unsigned int tmp_off_2=0;
  long unsigned int list1_off=0;
  int tmp_sgn, num1, num2;
  double complex tmp_V;
  double complex dam_pr;
  long int i_max;
  
  //For TPQ
  int step=0;
  int rand_i=0;
  //For Kond
  double complex dmv;

  
  i_max=X->Check.idim_max;
  X->Large.mode=M_CORR;
  tmp_V    = 1.0+0.0*I;
  
  if(GetSplitBitByModel(X->Def.Nsite, X->Def.iCalcModel, &irght, &ilft, &ihfbit)!=0){
    return -1;
  }
 
  dam_pr=0.0;

  //Make File Name for output
  switch (X->Def.iCalcType){
  case Lanczos:
    if(X->Def.St==0){
      sprintf(sdt, cFileName2BGreen_Lanczos, X->Def.CDataFileHead);
      TimeKeeper(X, cFileNameTimeKeep, cLanczosExpecTwoBodyGStart,"a");
      fprintf(stdoutMPI, cLogLanczosExpecTwoBodyGStart);
    }else if(X->Def.St==1){
      sprintf(sdt, cFileName2BGreen_CG, X->Def.CDataFileHead);
        TimeKeeper(X, cFileNameTimeKeep, cCGExpecTwoBodyGStart,"a");
      fprintf(stdoutMPI, cLogLanczosExpecTwoBodyGStart);
    }
    break;

  case TPQCalc:
    step=X->Def.istep;
    rand_i=X->Def.irand;
    TimeKeeperWithRandAndStep(X, cFileNameTimeKeep, cTPQExpecTwoBodyGStart, "a", rand_i, step);
    sprintf(sdt, cFileName2BGreen_TPQ, X->Def.CDataFileHead, rand_i, step);
    break;

  case FullDiag:
    sprintf(sdt, cFileName2BGreen_FullDiag, X->Def.CDataFileHead, X->Phys.eigen_num);
    break;
  }

  if(!childfopenMPI(sdt, "w", &fp)==0){
    return -1;
  }


  switch(X->Def.iCalcModel){
  case HubbardGC:
    for(i=0;i<X->Def.NCisAjtCkuAlvDC;i++){
      org_isite1   = X->Def.CisAjtCkuAlvDC[i][0]+1;
      org_sigma1   = X->Def.CisAjtCkuAlvDC[i][1];
      org_isite2   = X->Def.CisAjtCkuAlvDC[i][2]+1;
      org_sigma2   = X->Def.CisAjtCkuAlvDC[i][3];
      org_isite3   = X->Def.CisAjtCkuAlvDC[i][4]+1;
      org_sigma3   = X->Def.CisAjtCkuAlvDC[i][5];
      org_isite4   = X->Def.CisAjtCkuAlvDC[i][6]+1;
      org_sigma4   = X->Def.CisAjtCkuAlvDC[i][7];
      dam_pr=0.0;
      
      if(CheckPE(org_isite1-1, X)==TRUE || CheckPE(org_isite2-1, X)==TRUE ||
         CheckPE(org_isite3-1, X)==TRUE || CheckPE(org_isite4-1, X)==TRUE){
        isite1 = X->Def.OrgTpow[2*org_isite1-2+org_sigma1] ;
        isite2 = X->Def.OrgTpow[2*org_isite2-2+org_sigma2] ;
        isite3 = X->Def.OrgTpow[2*org_isite3-2+org_sigma3] ;
        isite4 = X->Def.OrgTpow[2*org_isite4-2+org_sigma4] ;
        if(isite1 == isite2 && isite3 == isite4){

          dam_pr = X_GC_child_CisAisCjtAjt_Hubbard_MPI(org_isite1-1, org_sigma1, 
                                                       org_isite3-1, org_sigma3, 
                                                       1.0, X, vec, vec);
        }
        else if(isite1 == isite2 && isite3 != isite4){
	  
          dam_pr = X_GC_child_CisAisCjtAku_Hubbard_MPI(org_isite1-1, org_sigma1, 
                                                       org_isite3-1, org_sigma3, org_isite4-1, org_sigma4,
                                                       1.0, X, vec, vec);
	  
        }
        else if(isite1 != isite2 && isite3 == isite4){
	
          dam_pr = X_GC_child_CisAjtCkuAku_Hubbard_MPI(org_isite1-1, org_sigma1, org_isite2-1, org_sigma2,
                                                       org_isite3-1, org_sigma3, 
                                                       1.0, X, vec, vec);
	  
        }
        else if(isite1 != isite2 && isite3 != isite4){
          dam_pr = X_GC_child_CisAjtCkuAlv_Hubbard_MPI(org_isite1-1, org_sigma1, org_isite2-1, org_sigma2,
                                                       org_isite3-1, org_sigma3, org_isite4-1, org_sigma4,
                                                       1.0, X, vec, vec);
        }
      
      }//InterPE
      else{
        child_general_int_GetInfo
          (
           i, 
           X,
           org_isite1,
           org_isite2,
           org_isite3,
           org_isite4,
           org_sigma1,
           org_sigma2,
           org_sigma3,
           org_sigma4,
           tmp_V
           );
	
        i_max  = X->Large.i_max;
        isite1 = X->Large.is1_spin;
        isite2 = X->Large.is2_spin;
        Asum   = X->Large.isA_spin;
        Adiff  = X->Large.A_spin;
	
        isite3 = X->Large.is3_spin;
        isite4 = X->Large.is4_spin;
        Bsum   = X->Large.isB_spin;
        Bdiff  = X->Large.B_spin;
 
        if(isite1 == isite2 && isite3 == isite4){
          dam_pr = 0.0;
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, tmp_sgn, dmv) firstprivate(i_max,X,isite1,isite2,isite4,isite3,Asum,Bsum,Adiff,Bdiff,tmp_off,tmp_off_2,tmp_V) shared(vec)
          for(j=1;j<=i_max;j++){
            dam_pr += GC_child_CisAisCisAis_element(j, isite1, isite3, tmp_V, vec, vec, X, &tmp_off);
          }
        }else if(isite1 == isite2 && isite3 != isite4){
          dam_pr = 0.0;
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, tmp_sgn, dmv) firstprivate(i_max,X,isite1,isite2,isite4,isite3,Asum,Bsum,Adiff,Bdiff,tmp_off,tmp_off_2,tmp_V) shared(vec)
          for(j=1;j<=i_max;j++){
            dam_pr += GC_child_CisAisCjtAku_element(j, isite1, isite3, isite4, Bsum, Bdiff, tmp_V, vec, vec, X, &tmp_off);
          }
        }else if(isite1 != isite2 && isite3 == isite4){
          dam_pr = 0.0;
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, tmp_sgn, dmv) firstprivate(i_max,X,isite1,isite2,isite4,isite3,Asum,Bsum,Adiff,Bdiff,tmp_off,tmp_off_2,tmp_V) shared(vec)
          for(j=1;j<=i_max;j++){
            dam_pr +=GC_child_CisAjtCkuAku_element(j, isite1, isite2, isite3, Asum, Adiff, tmp_V, vec, vec, X, &tmp_off);
          } 
        
        }else if(isite1 != isite2 && isite3 != isite4){
          dam_pr = 0.0;
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, tmp_sgn, dmv) firstprivate(i_max,X,isite1,isite2,isite4,isite3,Asum,Bsum,Adiff,Bdiff,tmp_off,tmp_off_2,tmp_V) shared(vec)
          for(j=1;j<=i_max;j++){
            dam_pr +=GC_child_CisAjtCkuAlv_element(j, isite1, isite2, isite3, isite4, Asum, Adiff, Bsum, Bdiff, tmp_V, vec, vec, X, &tmp_off_2);  
          }
        }
      }
      
      dam_pr = SumMPI_dc(dam_pr);
      fprintf(fp," %4ld %4ld %4ld %4ld %4ld %4ld %4ld %4ld %.10lf %.10lf\n",org_isite1-1,org_sigma1, org_isite2-1,org_sigma2, org_isite3-1, org_sigma3, org_isite4-1,org_sigma4, creal(dam_pr), cimag(dam_pr));
      
    }//Intra PE
      
    
    break;
 
  case KondoGC:
  case Hubbard:
  case Kondo:
    for(i=0;i<X->Def.NCisAjtCkuAlvDC;i++){
      org_isite1   = X->Def.CisAjtCkuAlvDC[i][0]+1;
      org_sigma1   = X->Def.CisAjtCkuAlvDC[i][1];
      org_isite2   = X->Def.CisAjtCkuAlvDC[i][2]+1;
      org_sigma2   = X->Def.CisAjtCkuAlvDC[i][3];
      org_isite3   = X->Def.CisAjtCkuAlvDC[i][4]+1;
      org_sigma3   = X->Def.CisAjtCkuAlvDC[i][5];
      org_isite4   = X->Def.CisAjtCkuAlvDC[i][6]+1;
      org_sigma4   = X->Def.CisAjtCkuAlvDC[i][7];
      tmp_V    = 1.0;

      dam_pr=0.0;
      if(X->Def.iFlgSzConserved ==TRUE){
        if(org_sigma1+org_sigma3 != org_sigma2+org_sigma4){
          dam_pr=SumMPI_dc(dam_pr);
          fprintf(fp," %4ld %4ld %4ld %4ld %4ld %4ld %4ld %4ld %.10lf %.10lf \n",org_isite1-1, org_sigma1, org_isite2-1, org_sigma2, org_isite3-1, org_sigma3, org_isite4-1, org_sigma4, creal(dam_pr), cimag(dam_pr));
          continue;
        }
      }

      if(CheckPE(org_isite1-1, X)==TRUE || CheckPE(org_isite2-1, X)==TRUE ||
         CheckPE(org_isite3-1, X)==TRUE || CheckPE(org_isite4-1, X)==TRUE){
        isite1 = X->Def.OrgTpow[2*org_isite1-2+org_sigma1] ;
        isite2 = X->Def.OrgTpow[2*org_isite2-2+org_sigma2] ;
        isite3 = X->Def.OrgTpow[2*org_isite3-2+org_sigma3] ;
        isite4 = X->Def.OrgTpow[2*org_isite4-2+org_sigma4] ;
        if(isite1 == isite2 && isite3 == isite4){
	  
          dam_pr = X_child_CisAisCjtAjt_Hubbard_MPI(org_isite1-1, org_sigma1, 
                                                    org_isite3-1, org_sigma3, 
                                                    1.0, X, vec, vec);
        }
        else if(isite1 == isite2 && isite3 != isite4){
	  	  
          dam_pr = X_child_CisAisCjtAku_Hubbard_MPI(org_isite1-1, org_sigma1, 
                                                    org_isite3-1, org_sigma3, org_isite4-1, org_sigma4,
                                                    1.0, X, vec, vec);
	   
        }
        else if(isite1 != isite2 && isite3 == isite4){
	  
          dam_pr = X_child_CisAjtCkuAku_Hubbard_MPI(org_isite1-1, org_sigma1, org_isite2-1, org_sigma2,
                                                    org_isite3-1, org_sigma3, 
                                                    1.0, X, vec, vec);
	 
        }
        else if(isite1 != isite2 && isite3 != isite4){	  
          dam_pr = X_child_CisAjtCkuAlv_Hubbard_MPI(org_isite1-1, org_sigma1, org_isite2-1, org_sigma2,
                                                    org_isite3-1, org_sigma3, org_isite4-1, org_sigma4,
                                                    1.0, X, vec, vec);
	  
        }
      
      }//InterPE
      else{
        child_general_int_GetInfo(
                                  i, 
                                  X,
                                  org_isite1,
                                  org_isite2,
                                  org_isite3,
                                  org_isite4,
                                  org_sigma1,
                                  org_sigma2,
                                  org_sigma3,
                                  org_sigma4,
                                  tmp_V
                                  );

        i_max  = X->Large.i_max;
        isite1 = X->Large.is1_spin;
        isite2 = X->Large.is2_spin;
        Asum   = X->Large.isA_spin;
        Adiff  = X->Large.A_spin;

        isite3 = X->Large.is3_spin;
        isite4 = X->Large.is4_spin;
        Bsum   = X->Large.isB_spin;
        Bdiff  = X->Large.B_spin;

        tmp_V  = 1.0;
        dam_pr = 0.0;
        if(isite1 == isite2 && isite3 == isite4){
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, tmp_sgn, dmv) firstprivate(i_max,X,isite1,isite2,isite4,isite3,Asum,Bsum,Adiff,Bdiff,tmp_off,tmp_off_2) shared(vec,tmp_V)
          for(j=1;j<=i_max;j++){
            dam_pr += child_CisAisCisAis_element(j, isite1, isite3, tmp_V, vec, vec, X, &tmp_off);
          }
        }else if(isite1 == isite2 && isite3 != isite4){
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, tmp_sgn, dmv) firstprivate(i_max,X,isite1,isite2,isite4,isite3,Asum,Bsum,Adiff,Bdiff,tmp_off,tmp_off_2) shared(vec,tmp_V)
          for(j=1;j<=i_max;j++){
            dam_pr += child_CisAisCjtAku_element(j, isite1, isite3, isite4, Bsum, Bdiff, tmp_V, vec, vec, X, &tmp_off);
          }
        }else if(isite1 != isite2 && isite3 == isite4){
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, tmp_sgn, dmv) firstprivate(i_max,X,isite1,isite2,isite4,isite3,Asum,Bsum,Adiff,Bdiff,tmp_off,tmp_off_2) shared(vec,tmp_V)
          for(j=1;j<=i_max;j++){
            dam_pr +=child_CisAjtCkuAku_element(j, isite1, isite2, isite3, Asum, Adiff, tmp_V, vec, vec, X, &tmp_off);
          } 
        }else if(isite1 != isite2 && isite3 != isite4){
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, tmp_sgn, dmv) firstprivate(i_max,X,isite1,isite2,isite4,isite3,Asum,Bsum,Adiff,Bdiff,tmp_off,tmp_off_2) shared(vec,tmp_V)
          for(j=1;j<=i_max;j++){
            dam_pr +=child_CisAjtCkuAlv_element(j, isite1, isite2, isite3, isite4, Asum, Adiff, Bsum, Bdiff, tmp_V, vec, vec, X, &tmp_off_2);
	  
          } 
        }    
      }
      dam_pr = SumMPI_dc(dam_pr);
      fprintf(fp," %4ld %4ld %4ld %4ld %4ld %4ld %4ld %4ld %.10lf %.10lf\n",org_isite1-1,org_sigma1, org_isite2-1,org_sigma2, org_isite3-1, org_sigma3, org_isite4-1,org_sigma4, creal(dam_pr), cimag(dam_pr));
    }
    break;
  
  case Spin:
    if(X->Def.iFlgGeneralSpin==FALSE){

      for(i=0;i<X->Def.NCisAjtCkuAlvDC;i++){
        tmp_org_isite1   = X->Def.CisAjtCkuAlvDC[i][0]+1;
        tmp_org_sigma1   = X->Def.CisAjtCkuAlvDC[i][1];
        tmp_org_isite2   = X->Def.CisAjtCkuAlvDC[i][2]+1;
        tmp_org_sigma2   = X->Def.CisAjtCkuAlvDC[i][3];
        tmp_org_isite3   = X->Def.CisAjtCkuAlvDC[i][4]+1;
        tmp_org_sigma3   = X->Def.CisAjtCkuAlvDC[i][5];
        tmp_org_isite4   = X->Def.CisAjtCkuAlvDC[i][6]+1;
        tmp_org_sigma4   = X->Def.CisAjtCkuAlvDC[i][7];

        if(Rearray_Interactions(i, &org_isite1, &org_isite2, &org_isite3, &org_isite4, &org_sigma1, &org_sigma2, &org_sigma3, &org_sigma4, &tmp_V, X)!=0){
          //error message will be added 
          fprintf(fp," %4ld %4ld %4ld %4ld %4ld %4ld %4ld %4ld %.10lf %.10lf \n",tmp_org_isite1-1, tmp_org_sigma1, tmp_org_isite2-1, tmp_org_sigma2, tmp_org_isite3-1,tmp_org_sigma3, tmp_org_isite4-1, tmp_org_sigma4,0.0,0.0);
          continue;
        }

        dam_pr = 0.0;
        if(org_isite1 >X->Def.Nsite && org_isite3>X->Def.Nsite){
          if(org_sigma1==org_sigma2 && org_sigma3==org_sigma4 ){ //diagonal
            is1_up = X->Def.Tpow[org_isite1 - 1];
            is2_up = X->Def.Tpow[org_isite3 - 1];
            num1 = X_SpinGC_CisAis((unsigned long int)myrank + 1, X, is1_up, org_sigma1);
            num2 = X_SpinGC_CisAis((unsigned long int)myrank + 1, X, is2_up, org_sigma3);
#pragma omp parallel for default(none) reduction (+:dam_pr) shared(vec) \
  firstprivate(i_max, num1, num2, tmp_V) private(j)
            for (j = 1; j <= i_max; j++) {
              dam_pr += tmp_V*num1*num2*vec[j]*conj(vec[j]);
            } 
          }
          else if(org_isite1==org_isite3 && org_sigma1==org_sigma4 && org_sigma2==org_sigma3){
            is1_up = X->Def.Tpow[org_isite1 - 1];
            num1 = X_SpinGC_CisAis((unsigned long int)myrank + 1, X, is1_up, org_sigma1);
#pragma omp parallel for default(none) reduction (+:dam_pr) shared(vec) \
  firstprivate(i_max, num1, num2, tmp_V) private(j)
            for (j = 1; j <= i_max; j++) {
              dam_pr += tmp_V*num1*vec[j]*conj(vec[j]);
            }
          }
          else if(org_sigma1==org_sigma4 && org_sigma2==org_sigma3){//exchange
            dam_pr += X_child_general_int_spin_MPIdouble(org_isite1-1, org_sigma1, org_sigma2, org_isite3-1, org_sigma3, org_sigma4, tmp_V, X, vec, vec);
          }
          else{  // other process is not allowed
            // error message will be added
          }
        }
        else if(org_isite1 > X->Def.Nsite || org_isite3>X->Def.Nsite){
          if(org_sigma1==org_sigma2 && org_sigma3==org_sigma4 ){ //diagonal
            is1_up = X->Def.Tpow[org_isite1 - 1];
            is2_up = X->Def.Tpow[org_isite3 - 1];
            num2 = X_SpinGC_CisAis((unsigned long int)myrank + 1, X, is2_up, org_sigma3);
            dam_pr=0.0;
#pragma omp parallel for default(none) reduction(+:dam_pr)shared(vec)	\
  firstprivate(i_max, tmp_V, is1_up, org_sigma1, X, num2) private(j, num1)
            for (j = 1; j <= i_max; j++) {
              num1 = X_Spin_CisAis(j, X, is1_up, org_sigma1);
              dam_pr += tmp_V*num1*num2*conj(vec[j])*vec[j];
            }
          }
          else if(org_sigma1==org_sigma4 && org_sigma2==org_sigma3){//exchange
            dam_pr += X_child_general_int_spin_MPIsingle(org_isite1-1, org_sigma1, org_sigma2, org_isite3-1, org_sigma3, org_sigma4, tmp_V, X, vec, vec);
          }
          else{  // other process is not allowed
            // error message will be added
            dam_pr=0.0;
          }	
        }
        else{
          isA_up = X->Def.Tpow[org_isite1-1];
          isB_up = X->Def.Tpow[org_isite3-1];
          if(org_sigma1==org_sigma2 && org_sigma3==org_sigma4 ){ //diagonal
            dam_pr = 0.0;
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j) firstprivate(i_max,X,isA_up,isB_up,org_sigma2,org_sigma4,tmp_off,tmp_off_2, tmp_V) shared(vec)
            for(j=1;j<=i_max;j++){
              dam_pr +=child_CisAisCisAis_spin_element(j, isA_up, isB_up, org_sigma2, org_sigma4, tmp_V, vec, vec, X);
            }
          }else if(org_isite1==org_isite3 && org_sigma1==org_sigma4 && org_sigma3==org_sigma2){
            dam_pr = 0.0;
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, dmv) firstprivate(i_max,X,isA_up, tmp_V) shared(vec, list_1)
            for(j=1;j<=i_max;j++){
              dmv=X_CisAis(list_1[j], X, isA_up);
              dam_pr += vec[j]*tmp_V*dmv*conj(vec[j]);
            }	    
          }
          else if(org_sigma1==org_sigma4 && org_sigma2==org_sigma3){ // exchange
            dam_pr = 0.0;
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, tmp_sgn, dmv) firstprivate(i_max,X,isA_up,isB_up,org_sigma2,org_sigma4,tmp_off,tmp_off_2,tmp_V) shared(vec)
            for(j=1;j<=i_max;j++){
              tmp_sgn    =  X_child_exchange_spin_element(j,X,isA_up,isB_up,org_sigma2,org_sigma4,&tmp_off);
              dmv        = vec[j]*tmp_sgn;
              dam_pr    += conj(vec[tmp_off])*dmv;
            }
          }else{  // other process is not allowed
            // error message will be added
            dam_pr=0.0;
          }	
        }
        dam_pr = SumMPI_dc(dam_pr);
        fprintf(fp," %4ld %4ld %4ld %4ld %4ld %4ld %4ld %4ld %.10lf %.10lf \n",tmp_org_isite1-1, tmp_org_sigma1, tmp_org_isite2-1, tmp_org_sigma2, tmp_org_isite3-1, tmp_org_sigma3, tmp_org_isite4-1, tmp_org_sigma4,creal(dam_pr),cimag(dam_pr));

      }
    }//iFlgGeneralSpin = FALSE
    else{
      for(i=0;i<X->Def.NCisAjtCkuAlvDC;i++){
        tmp_org_isite1   = X->Def.CisAjtCkuAlvDC[i][0]+1;
        tmp_org_sigma1   = X->Def.CisAjtCkuAlvDC[i][1];
        tmp_org_isite2   = X->Def.CisAjtCkuAlvDC[i][2]+1;
        tmp_org_sigma2   = X->Def.CisAjtCkuAlvDC[i][3];
        tmp_org_isite3   = X->Def.CisAjtCkuAlvDC[i][4]+1;
        tmp_org_sigma3   = X->Def.CisAjtCkuAlvDC[i][5];
        tmp_org_isite4   = X->Def.CisAjtCkuAlvDC[i][6]+1;
        tmp_org_sigma4   = X->Def.CisAjtCkuAlvDC[i][7];

        if(Rearray_Interactions(i, &org_isite1, &org_isite2, &org_isite3, &org_isite4, &org_sigma1, &org_sigma2, &org_sigma3, &org_sigma4, &tmp_V, X)!=0){
          fprintf(fp," %4ld %4ld %4ld %4ld %4ld %4ld %4ld %4ld %.10lf %.10lf \n",tmp_org_isite1-1, tmp_org_sigma1, tmp_org_isite2-1, tmp_org_sigma2, tmp_org_isite3-1,tmp_org_sigma3, tmp_org_isite4-1, tmp_org_sigma4,0.0,0.0);
          continue;
        }

        dam_pr = 0.0;
        if(org_isite1 >X->Def.Nsite && org_isite3>X->Def.Nsite){
          if(org_sigma1==org_sigma2 && org_sigma3==org_sigma4 ){ //diagonal
            dam_pr=X_child_CisAisCjuAju_GeneralSpin_MPIdouble(org_isite1-1, org_sigma1, org_isite3-1, org_sigma3, tmp_V, X, vec, vec);
          }
          else if(org_sigma1 != org_sigma2 && org_sigma3 != org_sigma4){
            dam_pr=X_child_CisAitCjuAjv_GeneralSpin_MPIdouble(org_isite1-1, org_sigma1, org_sigma2, org_isite3-1, org_sigma3, org_sigma4,tmp_V, X, vec, vec);
          }
          else{
            dam_pr=0.0;
          }
        }
        else if(org_isite3 > X->Def.Nsite || org_isite1 > X->Def.Nsite){
          if(org_sigma1==org_sigma2 && org_sigma3==org_sigma4 ){ //diagonal
            dam_pr=X_child_CisAisCjuAju_GeneralSpin_MPIsingle(org_isite1-1, org_sigma1, org_isite3-1, org_sigma3, tmp_V, X, vec, vec);
          }
          else if(org_sigma1 != org_sigma2 && org_sigma3 != org_sigma4){
            dam_pr=X_child_CisAitCjuAjv_GeneralSpin_MPIsingle(org_isite1-1, org_sigma1, org_sigma2, org_isite3-1, org_sigma3, org_sigma4,tmp_V, X, vec, vec);
          }
          else{
            dam_pr=0.0;
          }
        }
        else{
          if(org_sigma1==org_sigma2 && org_sigma3==org_sigma4 ){ //diagonal
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, num1) firstprivate(i_max,X,org_isite1, org_sigma1,org_isite3, org_sigma3, tmp_V) shared(vec,list_1)
            for(j=1;j<=i_max;j++){
              num1=BitCheckGeneral(list_1[j], org_isite1, org_sigma1, X->Def.SiteToBit, X->Def.Tpow);
              if(num1 != FALSE){
                num1=BitCheckGeneral(list_1[j], org_isite3, org_sigma3, X->Def.SiteToBit, X->Def.Tpow);
                if(num1 != FALSE){
                  dam_pr += tmp_V*conj(vec[j])*vec[j];
                }
              }
            }
          } 	   
          else if(org_sigma1 != org_sigma2 && org_sigma3 != org_sigma4){ 
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, num1) firstprivate(i_max,X, org_isite1, org_isite3, org_sigma1, org_sigma2, org_sigma3, org_sigma4, tmp_off, tmp_off_2, list1_off, tmp_V) shared(vec, list_1)
            for(j=1;j<=i_max;j++){
              num1 = num1*GetOffCompGeneralSpin(list_1[j], org_isite3, org_sigma4, org_sigma3, &tmp_off, X->Def.SiteToBit, X->Def.Tpow);
              if(num1 != FALSE){
                num1 = GetOffCompGeneralSpin(tmp_off, org_isite1, org_sigma2, org_sigma1, &tmp_off_2, X->Def.SiteToBit, X->Def.Tpow);
                ConvertToList1GeneralSpin(tmp_off_2, X->Check.sdim, &list1_off);
                if(num1 != FALSE){
                  dam_pr +=  tmp_V*conj(vec[list1_off])*vec[j];
                }
              }    
            }
          }
          else{
            dam_pr=0.0;
          }
        }
        dam_pr = SumMPI_dc(dam_pr);
        fprintf(fp," %4ld %4ld %4ld %4ld %4ld %4ld %4ld %4ld %.10lf %.10lf \n",tmp_org_isite1-1, tmp_org_sigma1, tmp_org_isite2-1, tmp_org_sigma2, tmp_org_isite3-1, tmp_org_sigma3, tmp_org_isite4-1, tmp_org_sigma4, creal(dam_pr),cimag(dam_pr));
      }      
    }
  
    break;

  case SpinGC:
    if(X->Def.iFlgGeneralSpin==FALSE){
      for(i=0;i<X->Def.NCisAjtCkuAlvDC;i++){
        tmp_org_isite1   = X->Def.CisAjtCkuAlvDC[i][0]+1;
        tmp_org_sigma1   = X->Def.CisAjtCkuAlvDC[i][1];
        tmp_org_isite2   = X->Def.CisAjtCkuAlvDC[i][2]+1;
        tmp_org_sigma2   = X->Def.CisAjtCkuAlvDC[i][3];
        tmp_org_isite3   = X->Def.CisAjtCkuAlvDC[i][4]+1;
        tmp_org_sigma3   = X->Def.CisAjtCkuAlvDC[i][5];
        tmp_org_isite4   = X->Def.CisAjtCkuAlvDC[i][6]+1;
        tmp_org_sigma4   = X->Def.CisAjtCkuAlvDC[i][7];
    
        if(Rearray_Interactions(i, &org_isite1, &org_isite2, &org_isite3, &org_isite4, &org_sigma1, &org_sigma2, &org_sigma3, &org_sigma4, &tmp_V, X)!=0){
          //error message will be added 
          fprintf(fp," %4ld %4ld %4ld %4ld %4ld %4ld %4ld %4ld %.10lf %.10lf \n",tmp_org_isite1-1, tmp_org_sigma1, tmp_org_isite2-1, tmp_org_sigma2, tmp_org_isite3-1,tmp_org_sigma3, tmp_org_isite4-1, tmp_org_sigma4,0.0,0.0);
          continue;
        }

        dam_pr=0.0;
        if(org_isite1>X->Def.Nsite && org_isite3>X->Def.Nsite){ //org_isite3 >= org_isite1 > Nsite

          if(org_sigma1==org_sigma2 && org_sigma3==org_sigma4 ){ //diagonal	    
            dam_pr += X_GC_child_CisAisCjuAju_spin_MPIdouble( (org_isite1-1), org_sigma1, (org_isite3-1), org_sigma3, tmp_V, X, vec, vec);
	    
          }
          else if(org_isite1 ==org_isite3 && org_sigma1 ==org_sigma4 && org_sigma2 ==org_sigma3){ //diagonal (for spin: cuadcdau=cuau)
            dam_pr += X_GC_child_CisAis_spin_MPIdouble((org_isite1-1), org_sigma1, tmp_V, X, vec, vec);	    	    
          }
          else if(org_sigma1 == org_sigma2 && org_sigma3 != org_sigma4){
            dam_pr += X_GC_child_CisAisCjuAjv_spin_MPIdouble(org_isite1-1, org_sigma1, org_isite3-1, org_sigma3, org_sigma4, tmp_V, X, vec, vec);
          }
          else if(org_sigma1 != org_sigma2 && org_sigma3 == org_sigma4){
            dam_pr += X_GC_child_CisAitCjuAju_spin_MPIdouble(org_isite1-1, org_sigma1, org_sigma2, org_isite3-1, org_sigma3, tmp_V, X, vec, vec);
          }
          else if(org_sigma1 != org_sigma2 && org_sigma3 != org_sigma4){
            dam_pr +=  X_GC_child_CisAitCiuAiv_spin_MPIdouble(org_isite1-1, org_sigma1, org_sigma2, org_isite3-1, org_sigma3, org_sigma4, tmp_V, X, vec, vec);
          } 
        }
        else if(org_isite3>X->Def.Nsite || org_isite1>X->Def.Nsite){ //org_isite3 > Nsite >= org_isite1 
          if(org_sigma1==org_sigma2 && org_sigma3==org_sigma4 ){ //diagonal
            dam_pr += X_GC_child_CisAisCjuAju_spin_MPIsingle( (org_isite1-1), org_sigma1, (org_isite3-1), org_sigma3, tmp_V, X, vec, vec);	    

          }
          else if(org_sigma1 == org_sigma2 && org_sigma3 != org_sigma4){
            dam_pr += X_GC_child_CisAisCjuAjv_spin_MPIsingle(org_isite1-1, org_sigma1, org_isite3-1, org_sigma3, org_sigma4, tmp_V, X, vec, vec);
          }
          else if(org_sigma1 != org_sigma2 && org_sigma3 == org_sigma4){
            dam_pr += X_GC_child_CisAitCjuAju_spin_MPIsingle(org_isite1-1, org_sigma1, org_sigma2, org_isite3-1, org_sigma3, tmp_V, X, vec, vec);
          }
          else if(org_sigma1 != org_sigma2 && org_sigma3 != org_sigma4){
            dam_pr +=  X_GC_child_CisAitCiuAiv_spin_MPIsingle(org_isite1-1, org_sigma1, org_sigma2, org_isite3-1, org_sigma3, org_sigma4, tmp_V, X, vec, vec);
          }
        }
        else{	
          if(org_isite1==org_isite2 && org_isite3==org_isite4){
            isA_up = X->Def.Tpow[org_isite2-1];
            isB_up = X->Def.Tpow[org_isite4-1];
            if(org_sigma1==org_sigma2 && org_sigma3==org_sigma4 ){ //diagonal
              dam_pr = 0.0;
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, tmp_sgn, dmv) firstprivate(i_max,X,isA_up,isB_up,org_sigma2,org_sigma4,tmp_off,tmp_off_2,tmp_V) shared(vec)
              for(j=1;j<=i_max;j++){
                dam_pr +=GC_child_CisAisCisAis_spin_element(j, isA_up, isB_up, org_sigma2, org_sigma4, tmp_V, vec, vec, X);
              }
            }else if(org_sigma1 == org_sigma2 && org_sigma3 != org_sigma4){ 
              dam_pr = 0.0;
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, tmp_sgn, dmv) firstprivate(i_max,X,isA_up,isB_up,org_sigma2,org_sigma4,tmp_off,tmp_off_2,tmp_V) shared(vec)
              for(j=1;j<=i_max;j++){
                dam_pr += GC_child_CisAisCitAiu_spin_element(j, org_sigma2, org_sigma4, isA_up, isB_up, tmp_V, vec, vec, X, &tmp_off);
              } 
            }else if(org_sigma1 != org_sigma2 && org_sigma3 == org_sigma4){ 
              dam_pr = 0.0;
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, tmp_sgn, dmv) firstprivate(i_max,X,isA_up,isB_up,org_sigma2,org_sigma4,tmp_off,tmp_off_2,tmp_V) shared(vec)
              for(j=1;j<=i_max;j++){
                dam_pr += GC_child_CisAitCiuAiu_spin_element(j, org_sigma2, org_sigma4, isA_up, isB_up, tmp_V, vec, vec, X, &tmp_off);
              } 
            }else if(org_sigma1 != org_sigma2 && org_sigma3 != org_sigma4){ 
              dam_pr = 0.0;
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, tmp_sgn, dmv) firstprivate(i_max,X,isA_up,isB_up,org_sigma2,org_sigma4,tmp_off,tmp_off_2,tmp_V) shared(vec)
              for(j=1;j<=i_max;j++){
                dam_pr += GC_child_CisAitCiuAiv_spin_element(j, org_sigma2, org_sigma4, isA_up, isB_up, tmp_V, vec, vec, X, &tmp_off);
              }
            }
          }
        }
        dam_pr = SumMPI_dc(dam_pr);
        fprintf(fp," %4ld %4ld %4ld %4ld %4ld %4ld %4ld %4ld %.10lf %.10lf \n",tmp_org_isite1-1, tmp_org_sigma1, tmp_org_isite2-1, tmp_org_sigma2, tmp_org_isite3-1, tmp_org_sigma3, tmp_org_isite4-1, tmp_org_sigma4,creal(dam_pr),cimag(dam_pr));
      }
    }
    else{
      for(i=0;i<X->Def.NCisAjtCkuAlvDC;i++){
   
        tmp_org_isite1   = X->Def.CisAjtCkuAlvDC[i][0]+1;
        tmp_org_sigma1   = X->Def.CisAjtCkuAlvDC[i][1];
        tmp_org_isite2   = X->Def.CisAjtCkuAlvDC[i][2]+1;
        tmp_org_sigma2   = X->Def.CisAjtCkuAlvDC[i][3];
        tmp_org_isite3   = X->Def.CisAjtCkuAlvDC[i][4]+1;
        tmp_org_sigma3   = X->Def.CisAjtCkuAlvDC[i][5];
        tmp_org_isite4   = X->Def.CisAjtCkuAlvDC[i][6]+1;
        tmp_org_sigma4   = X->Def.CisAjtCkuAlvDC[i][7];

        if(Rearray_Interactions(i, &org_isite1, &org_isite2, &org_isite3, &org_isite4, &org_sigma1, &org_sigma2, &org_sigma3, &org_sigma4, &tmp_V, X)!=0){
          //error message will be added 
          fprintf(fp," %4ld %4ld %4ld %4ld %4ld %4ld %4ld %4ld %.10lf %.10lf \n",tmp_org_isite1-1, tmp_org_sigma1, tmp_org_isite2-1, tmp_org_sigma2, tmp_org_isite3-1,tmp_org_sigma3, tmp_org_isite4-1, tmp_org_sigma4,0.0,0.0);
          continue;
        }
     
        dam_pr = 0.0;
        if(org_isite1 > X->Def.Nsite && org_isite3 > X->Def.Nsite){
          if(org_sigma1==org_sigma2 && org_sigma3==org_sigma4 ){ //diagonal
            dam_pr=X_GC_child_CisAisCjuAju_GeneralSpin_MPIdouble(org_isite1-1, org_sigma1, org_isite3-1, org_sigma3, tmp_V, X, vec, vec);
          }
          else if(org_sigma1 == org_sigma2 && org_sigma3 != org_sigma4){
            dam_pr=X_GC_child_CisAisCjuAjv_GeneralSpin_MPIdouble(org_isite1-1, org_sigma1, org_isite3-1, org_sigma3, org_sigma4, tmp_V, X, vec, vec);
          }
          else if(org_sigma1 != org_sigma2 && org_sigma3 == org_sigma4){
            dam_pr=X_GC_child_CisAitCjuAju_GeneralSpin_MPIdouble(org_isite1-1, org_sigma1, org_sigma2, org_isite3-1, org_sigma3, tmp_V, X, vec, vec);
          }
          else if(org_sigma1 != org_sigma2 && org_sigma3 != org_sigma4){
            dam_pr=X_GC_child_CisAitCjuAjv_GeneralSpin_MPIdouble(org_isite1-1, org_sigma1, org_sigma2, org_isite3-1, org_sigma3, org_sigma4,tmp_V, X, vec, vec);
          }
        }
        else if(org_isite3 > X->Def.Nsite || org_isite1 > X->Def.Nsite){
          if(org_sigma1==org_sigma2 && org_sigma3==org_sigma4 ){ //diagonal
            dam_pr=X_GC_child_CisAisCjuAju_GeneralSpin_MPIsingle(org_isite1-1, org_sigma1, org_isite3-1, org_sigma3, tmp_V, X, vec, vec);
          }
          else if(org_sigma1 == org_sigma2 && org_sigma3 != org_sigma4){
            dam_pr=X_GC_child_CisAisCjuAjv_GeneralSpin_MPIsingle(org_isite1-1, org_sigma1, org_isite3-1, org_sigma3, org_sigma4, tmp_V, X, vec, vec);
          }
          else if(org_sigma1 != org_sigma2 && org_sigma3 == org_sigma4){
            dam_pr=X_GC_child_CisAitCjuAju_GeneralSpin_MPIsingle(org_isite1-1, org_sigma1, org_sigma2, org_isite3-1, org_sigma3, tmp_V, X, vec, vec);
          }
          else if(org_sigma1 != org_sigma2 && org_sigma3 != org_sigma4){
            dam_pr=X_GC_child_CisAitCjuAjv_GeneralSpin_MPIsingle(org_isite1-1, org_sigma1, org_sigma2, org_isite3-1, org_sigma3, org_sigma4,tmp_V, X, vec, vec);
          }
        }
        else{
          if(org_sigma1==org_sigma2 && org_sigma3==org_sigma4 ){ //diagonal
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, num1) firstprivate(i_max,X,org_isite1, org_sigma1,org_isite3, org_sigma3, tmp_V) shared(vec)
            for(j=1;j<=i_max;j++){
              num1=BitCheckGeneral(j-1, org_isite1, org_sigma1, X->Def.SiteToBit, X->Def.Tpow);
              if(num1 != FALSE){
                num1=BitCheckGeneral(j-1, org_isite3, org_sigma3, X->Def.SiteToBit, X->Def.Tpow);
                if(num1 != FALSE){
                  dam_pr += tmp_V*conj(vec[j])*vec[j];
                }
              }
            }
          }else if(org_sigma1 == org_sigma2 && org_sigma3 != org_sigma4){ 
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, num1) firstprivate(i_max,X, org_isite1, org_isite3, org_sigma1,org_sigma3,org_sigma4, tmp_off, tmp_V) shared(vec)
            for(j=1;j<=i_max;j++){
              num1 = GetOffCompGeneralSpin(j-1, org_isite3, org_sigma4, org_sigma3, &tmp_off, X->Def.SiteToBit, X->Def.Tpow);
              if(num1 != FALSE){
                num1=BitCheckGeneral(tmp_off, org_isite1, org_sigma1, X->Def.SiteToBit, X->Def.Tpow);
                if(num1 != FALSE){
                  dam_pr += tmp_V*conj(vec[tmp_off+1])*vec[j];
                }
              }
            } 
          }else if(org_sigma1 != org_sigma2 && org_sigma3 == org_sigma4){ 
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, num1) firstprivate(i_max,X, org_isite1, org_isite3, org_sigma1,org_sigma2, org_sigma3, tmp_off, tmp_V) shared(vec)
            for(j=1;j<=i_max;j++){
              num1 = BitCheckGeneral(j-1, org_isite3, org_sigma3, X->Def.SiteToBit, X->Def.Tpow);
              if(num1 != FALSE){
                num1 = GetOffCompGeneralSpin(j-1, org_isite1, org_sigma2, org_sigma1, &tmp_off, X->Def.SiteToBit, X->Def.Tpow);
                if(num1 != FALSE){
                  dam_pr +=  tmp_V*conj(vec[tmp_off+1])*vec[j];
                }
              }
            } 	   
          }else if(org_sigma1 != org_sigma2 && org_sigma3 != org_sigma4){ 
#pragma omp parallel for default(none) reduction(+:dam_pr) private(j, num1) firstprivate(i_max,X, org_isite1, org_isite3, org_sigma1, org_sigma2, org_sigma3, org_sigma4, tmp_off, tmp_off_2, tmp_V) shared(vec)
            for(j=1;j<=i_max;j++){
              num1 = GetOffCompGeneralSpin(j-1, org_isite3, org_sigma4, org_sigma3, &tmp_off, X->Def.SiteToBit, X->Def.Tpow);
              if(num1 != FALSE){
                num1 = GetOffCompGeneralSpin(tmp_off, org_isite1, org_sigma2, org_sigma1, &tmp_off_2, X->Def.SiteToBit, X->Def.Tpow);
                if(num1 != FALSE){
                  dam_pr +=  tmp_V*conj(vec[tmp_off_2+1])*vec[j];
                }
              }
	     
            }
          }
        }
        dam_pr = SumMPI_dc(dam_pr);
        fprintf(fp," %4ld %4ld %4ld %4ld %4ld %4ld %4ld %4ld %.10lf %.10lf \n",tmp_org_isite1-1, tmp_org_sigma1, tmp_org_isite2-1, tmp_org_sigma2, tmp_org_isite3-1, tmp_org_sigma3, tmp_org_isite4-1, tmp_org_sigma4, creal(dam_pr),cimag(dam_pr));     
      }
    }
    break;    

  default:
    return -1;
  }
  
  fclose(fp);
  
  if(X->Def.iCalcType==Lanczos){
    if(X->Def.St==0){
      TimeKeeper(X, cFileNameTimeKeep, cLanczosExpecTwoBodyGFinish,"a");
      fprintf(stdoutMPI, "%s", cLogLanczosExpecTwoBodyGFinish);
    }else if(X->Def.St==1){
      TimeKeeper(X, cFileNameTimeKeep, cCGExpecTwoBodyGFinish,"a");
      fprintf(stdoutMPI, "%s", cLogCGExpecTwoBodyGFinish);
    }
  }
  else if(X->Def.iCalcType==TPQCalc){
    TimeKeeperWithRandAndStep(X, cFileNameTimeKeep, cTPQExpecTwoBodyGFinish, "a", rand_i, step);
  }
  //[s] this part will be added
  /* For FullDiag, it is convinient to calculate the total spin for each vector.
     Such functions will be added 
     if(X->Def.iCalcType==FullDiag){
     if(X->Def.iCalcModel==Spin){
     expec_cisajscktaltdc_alldiag_spin(X,vec);
     }else if(X->Def.iCalcModel==Hubbard || X->Def.iCalcModel==Kondo){
     expec_cisajscktaltdc_alldiag(X,vec);
     }else{// 
     X->Phys.s2=0.0;   
     }
     }
  */
  //[e]
  return 0;
}


/** 
 * @note Not Used now
 * 
 * @param X 
 * @param vec 
 * @author Takahiro Misawa (The University of Tokyo)
 * @author Kazuyoshi Yoshimi (The University of Tokyo)
 */
void expec_cisajscktaltdc_alldiag(struct BindStruct *X,double complex *vec){ // only for Spin and Hubbard

  long unsigned int j;
  long unsigned int irght,ilft,ihfbit;
  long unsigned int isite1,isite2;
  long unsigned int is1_up,is2_up,is1_down,is2_down;
  long unsigned int iexchg, off;
  int num1_up,num2_up;
  int num1_down,num2_down; 
  long unsigned int ibit1_up,ibit2_up,ibit1_down,ibit2_down; 
  double complex spn_z,chrg_z;
  double complex spn,chrg;
  double complex t_spn_z;  
  
  int N2,i_max;
    
  N2=2*X->Def.Nsite;
  i_max=X->Check.idim_max;
    
  irght=pow(2,((N2+1)/2))-1;
  ilft=pow(2,N2)-1;
  ilft=ilft ^ irght; 
  ihfbit=pow(2,((N2+1)/2));
  chrg=0.0;
  spn=0.0;
  t_spn_z=0.0;
  
  for(isite1=1;isite1<=X->Def.Nsite;isite1++){
    for(isite2=1;isite2<=X->Def.Nsite;isite2++){
            
      is1_up=X->Def.Tpow[2*isite1-2];
      is1_down=X->Def.Tpow[2*isite1-1];
      is2_up=X->Def.Tpow[2*isite2-2];
      is2_down=X->Def.Tpow[2*isite2-1];
      
#pragma omp parallel for reduction(+: t_spn_z,spn, chrg) firstprivate(i_max, is1_up, is2_up, is1_down, is2_down, irght, ilft, ihfbit) private(ibit1_up, num1_up, ibit2_up, num2_up, ibit1_down, num1_down, ibit2_down, num2_down, spn_z, chrg_z, iexchg, off)
      for(j=1;j<=i_max;j++){                    
        ibit1_up= list_1[j]&is1_up;
        num1_up=ibit1_up/is1_up;            
        ibit2_up= list_1[j]&is2_up;
        num2_up=ibit2_up/is2_up;
            
        ibit1_down= list_1[j]&is1_down;
        num1_down=ibit1_down/is1_down;
            
        ibit2_down= list_1[j]&is2_down;
        num2_down=ibit2_down/is2_down;
            
        spn_z=(num1_up-num1_down)*(num2_up-num2_down);
        chrg_z=(num1_up+num1_down)*(num2_up+num2_down);
            
        t_spn_z+= conj(vec[j])* vec[j]*(num1_up-num1_down);
        spn+= conj(vec[j])* vec[j]*spn_z;
        chrg+= conj(vec[j])* vec[j]*chrg_z;
            
        if(isite1==isite2){
          spn+=2* conj(vec[j])* vec[j]*(num1_up+num1_down-2*num1_up*num1_down);
        }else{
          if(ibit1_up!=0 && ibit1_down==0 && ibit2_up==0 &&ibit2_down!=0 ){
            iexchg= list_1[j]-(is1_up+is2_down);
            iexchg+=(is2_up+is1_down);
            GetOffComp(list_2_1, list_2_2, iexchg,  irght, ilft, ihfbit, &off);                    
            spn+=2* conj(vec[j])* vec[off];
          }else if(ibit1_up==0 && ibit1_down!=0 && ibit2_up!=0 && ibit2_down==0){
            iexchg= list_1[j]-(is1_down+is2_up);
            iexchg+=(is2_down+is1_up);
            GetOffComp(list_2_1, list_2_2, iexchg,  irght, ilft, ihfbit, &off);
            spn+=2* conj(vec[j])* vec[off];
          }
        }
      }
    }
  }
  spn = spn/X->Def.Nsite;
  X->Phys.s2=spn;
}


int Rearray_Interactions(
                         int i,
                         long unsigned int *org_isite1,
                         long unsigned int *org_isite2,
                         long unsigned int *org_isite3,
                         long unsigned int *org_isite4,
                         long unsigned int *org_sigma1,
                         long unsigned int *org_sigma2,
                         long unsigned int *org_sigma3,
                         long unsigned int *org_sigma4,
                         double complex *tmp_V, 
                         struct BindStruct *X
                         )
{
  long unsigned int tmp_org_isite1,tmp_org_isite2,tmp_org_isite3,tmp_org_isite4;
  long unsigned int tmp_org_sigma1,tmp_org_sigma2,tmp_org_sigma3,tmp_org_sigma4;
  
  tmp_org_isite1   = X->Def.CisAjtCkuAlvDC[i][0]+1;
  tmp_org_sigma1   = X->Def.CisAjtCkuAlvDC[i][1];
  tmp_org_isite2   = X->Def.CisAjtCkuAlvDC[i][2]+1;
  tmp_org_sigma2   = X->Def.CisAjtCkuAlvDC[i][3];
  tmp_org_isite3   = X->Def.CisAjtCkuAlvDC[i][4]+1;
  tmp_org_sigma3   = X->Def.CisAjtCkuAlvDC[i][5];
  tmp_org_isite4   = X->Def.CisAjtCkuAlvDC[i][6]+1;
  tmp_org_sigma4   = X->Def.CisAjtCkuAlvDC[i][7];
  
  if(tmp_org_isite1==tmp_org_isite2 && tmp_org_isite3==tmp_org_isite4){
    if(tmp_org_isite1 > tmp_org_isite3){
      *org_isite1   = tmp_org_isite3;
      *org_sigma1   = tmp_org_sigma3;
      *org_isite2   = tmp_org_isite4;
      *org_sigma2   = tmp_org_sigma4;
      *org_isite3   = tmp_org_isite1;
      *org_sigma3   = tmp_org_sigma1;
      *org_isite4   = tmp_org_isite2;
      *org_sigma4   = tmp_org_sigma2;
    }
    else{
      *org_isite1   = tmp_org_isite1;
      *org_sigma1   = tmp_org_sigma1;
      *org_isite2   = tmp_org_isite2;
      *org_sigma2   = tmp_org_sigma2;
      *org_isite3   = tmp_org_isite3;
      *org_sigma3   = tmp_org_sigma3;
      *org_isite4   = tmp_org_isite4;
      *org_sigma4   = tmp_org_sigma4;
    }
    *tmp_V = 1.0;
    
  }
  else if(tmp_org_isite1==tmp_org_isite4 && tmp_org_isite3==tmp_org_isite2){
    if(tmp_org_isite1 > tmp_org_isite3){
      *org_isite1   = tmp_org_isite3;
      *org_sigma1   = tmp_org_sigma3;
      *org_isite2   = tmp_org_isite2;
      *org_sigma2   = tmp_org_sigma2;
      *org_isite3   = tmp_org_isite1;
      *org_sigma3   = tmp_org_sigma1;
      *org_isite4   = tmp_org_isite4;
      *org_sigma4   = tmp_org_sigma4;
    }
    else{
      *org_isite1   = tmp_org_isite1;
      *org_sigma1   = tmp_org_sigma1;
      *org_isite2   = tmp_org_isite4;
      *org_sigma2   = tmp_org_sigma4;
      *org_isite3   = tmp_org_isite3;
      *org_sigma3   = tmp_org_sigma3;
      *org_isite4   = tmp_org_isite2;
      *org_sigma4   = tmp_org_sigma2;
    }
    *tmp_V =-1.0;
  }
  else{
    return -1;
  }
  return 0;
}
