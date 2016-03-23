/* HPhi  -  Quantum Lattice Model Simulator */
/* Copyright (C) 2015 Takahiro Misawa, Kazuyoshi Yoshimi, Mitsuaki Kawamura, Youhei Yamaji, Synge Todo, Naoki Kawashima */

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
#include "expec_cisajs.h"
#include "expec_cisajscktaltdc.h"
#include "expec_totalspin.h"
#include "CalcByLanczos.h"
#include "FileIO.h"
#include "wrapperMPI.h"

/**
 * @file   CalcByLanczos.c
 * @version 0.1, 0.2
 * @author Takahiro Misawa (The University of Tokyo)
 * @author Kazuyoshi Yoshimi (The University of Tokyo)
 * 
 * @brief  File for givinvg functions of calculating eigenvalues and eigenvectors by Lanczos method 
 * 
 * 
 */


/** 
 * @brief A main function to calculate eigenvalues and eigenvectors by Lanczos method 
 * 
 * @param[in,out] X CalcStruct list for getting and pushing calculation information 
 * @retval 0 normally finished
 * @retval -1 unnormally finished
 *
 * @version 0.2
 * @date 2015/10/20 add function of using a flag of iCalcEigenVec
 * @version 0.1
 * @author Takahiro Misawa (The University of Tokyo)
 * @author Kazuyoshi Yoshimi (The University of Tokyo)
 * 
 */
int CalcByLanczos(		 
		  struct EDMainCalStruct *X
				 )
{
  char sdt[D_FileNameMax];
  double diff_ene,var;
  long int i=0;
  long int libuf=0;
  long int i_max=0;
  long int _list_1;
  double dRealVec, dImagVec;
  FILE *fp;
  char ctmp2[256];
  
  if(X->Bind.Def.iInputEigenVec==FALSE){
    // this part will be modified
    switch(X->Bind.Def.iCalcModel){
    case HubbardGC:
    case SpinGC:
    case KondoGC:
      initial_mode = 1; // 1 -> random initial vector
      break;
    case Hubbard:
    case Kondo:
    case Spin:
      if(X->Bind.Def.iFlgGeneralSpin ==TRUE){
	initial_mode=1;
      }
      else{
	if(X->Bind.Def.initial_iv>0){
	  initial_mode = 0; // 0 -> only v[iv] = 1
	}else{
	  initial_mode = 1; // 1 -> random initial vector
	}
      }
      break;
    default:
      fclose(fp);
      exitMPI(-1);
    }
 
    if(Lanczos_EigenValue(&(X->Bind))!=0){
      fprintf(stderr, "  Lanczos Eigenvalue is not converged in this process.\n");      
      return(-1);
    }

    if(X->Bind.Def.iCalcEigenVec==CALCVEC_NOT){
       fprintf(stdoutMPI, "  Lanczos EigenValue = %.10lf \n ",X->Bind.Phys.Target_energy);
      return(0);
    }

    fprintf(stdoutMPI, cLogLanczos_EigenVecStart);
    
    if(X->Bind.Check.idim_maxMPI != 1){
      Lanczos_EigenVector(&(X->Bind));
      expec_energy(&(X->Bind));
      //check for the accuracy of the eigenvector
      var      = fabs(X->Bind.Phys.var-X->Bind.Phys.energy*X->Bind.Phys.energy)/fabs(X->Bind.Phys.var);
      diff_ene = fabs(X->Bind.Phys.Target_energy-X->Bind.Phys.energy)/fabs(X->Bind.Phys.Target_energy);
      
      fprintf(stdoutMPI, "\n");
      fprintf(stdoutMPI, "  Accuracy check !!!\n");
      fprintf(stdoutMPI, "  LanczosEnergy = %.14e \n  EnergyByVec   = %.14e \n  diff_ene      = %.14e \n  var           = %.14e \n",X->Bind.Phys.Target_energy,X->Bind.Phys.energy,diff_ene,var);
      if(diff_ene < eps_Energy && var< eps_Energy){
	fprintf(stdoutMPI, "  Accuracy of Lanczos vectors is enough.\n");
	fprintf(stdoutMPI, "\n");
      }else{
	/* Comment out: Power Lanczos method
	fprintf(stdoutMPI, "  Accuracy of Lanczos vectors is NOT enough\n");
	iconv=1;
	fprintf(stdoutMPI, "Eigenvector is improved by power Lanczos method \n");
	fprintf(stdoutMPI, "Power Lanczos starts\n");
	flag=PowerLanczos(&(X->Bind));
	fprintf(stdoutMPI, "Power Lanczos ends\n");
	if(flag==1){
	  var      = fabs(X->Bind.Phys.var-X->Bind.Phys.energy*X->Bind.Phys.energy)/fabs(X->Bind.Phys.var);
	  diff_ene = fabs(X->Bind.Phys.Target_energy-X->Bind.Phys.energy)/fabs(X->Bind.Phys.Target_energy);
	  fprintf(stdoutMPI,"\n");
	  fprintf(stdoutMPI,"Power Lanczos Accuracy check !!!\n");
	  fprintf(stdoutMPI,"%.14e %.14e: diff_ene=%.14e var=%.14e \n ",X->Bind.Phys.Target_energy,X->Bind.Phys.energy,diff_ene,var);
	  fprintf(stdoutMPI,"\n");
	
	}
	else if(X->Bind.Def.iCalcEigenVec==CALCVEC_LANCZOSCG && iconv==1){
	*/
	  fprintf(stdoutMPI, "  Accuracy of Lanczos vectors is NOT enough\n\n");
	  X->Bind.Def.St=1;
	  CG_EigenVector(&(X->Bind));
	  expec_energy(&(X->Bind));
	  var      = fabs(X->Bind.Phys.var-X->Bind.Phys.energy*X->Bind.Phys.energy)/fabs(X->Bind.Phys.var);
	  diff_ene = fabs(X->Bind.Phys.Target_energy-X->Bind.Phys.energy)/fabs(X->Bind.Phys.Target_energy);
	  fprintf(stdoutMPI, "\n");
	  fprintf(stdoutMPI, "  CG Accuracy check !!!\n");
	  fprintf(stdoutMPI, "  LanczosEnergy = %.14e\n  EnergyByVec   = %.14e\n  diff_ene      = %.14e\n  var           = %.14e \n ",X->Bind.Phys.Target_energy,X->Bind.Phys.energy,diff_ene,var);
	  fprintf(stdoutMPI, "\n");
	  //}
      }
    }
    else{//idim_max=1
      v0[1]=1;
      expec_energy(&(X->Bind));
    }
  }
  else{//input v1
    fprintf(stdoutMPI, "An Eigenvector is inputted.\n");
    sprintf(sdt, cFileNameInputEigen, X->Bind.Def.CDataFileHead, X->Bind.Def.k_exct-1, myrank);
    fp = fopen(sdt, "rb");
    if(fp==NULL){
      fprintf(stderr, "Error: A file of Inputvector does not exist.\n");
      fclose(fp);
      exitMPI(-1);
    }
    fread(&i_max, sizeof(long int), 1, fp);
    if(i_max != X->Bind.Check.idim_max){
      fprintf(stderr, "Error: A file of Inputvector is incorrect.\n");
      fclose(fp);
      exitMPI(-1);
    }
    fread(v1, sizeof(complex double),X->Bind.Check.idim_max+1, fp);
    
    fclose(fp);
  }

  fprintf(stdoutMPI, cLogLanczos_EigenVecEnd);
  // v1 is eigen vector
    
  if(!expec_cisajs(&(X->Bind), v1)==0){
    fprintf(stderr, "Error: calc OneBodyG.\n");
    exitMPI(-1);
  }
  
  if(!expec_cisajscktaltdc(&(X->Bind), v1)==0){
    fprintf(stderr, "Error: calc TwoBodyG.\n");
    exitMPI(-1);
  }
  
  /* For ver.1.0
  if(!expec_totalspin(&(X->Bind), v1)==0){
    fprintf(stderr, "Error: calc TotalSpin.\n");
    exitMPI(-1);
  }
  */
  
  if(!expec_totalSz(&(X->Bind), v1)==0){
    fprintf(stderr, "Error: calc TotalSz.\n");
    exitMPI(-1);
  }

  if(X->Bind.Def.St==0){
    sprintf(sdt, cFileNameEnergy_Lanczos, X->Bind.Def.CDataFileHead);
  }else if(X->Bind.Def.St==1){
    sprintf(sdt, cFileNameEnergy_CG, X->Bind.Def.CDataFileHead);
  }
  
  if(childfopenMPI(sdt, "w", &fp)!=0){
    exitMPI(-1);
  }  


  fprintf(fp,"Energy  %.16lf \n",X->Bind.Phys.energy);
  fprintf(fp,"Doublon  %.16lf \n",X->Bind.Phys.doublon);
  fprintf(fp,"Sz  %.16lf \n",X->Bind.Phys.sz);
  //    fprintf(fp,"total S^2  %.10lf \n",X->Bind.Phys.s2);    
  fclose(fp);

  if(X->Bind.Def.iOutputEigenVec==TRUE){
    sprintf(sdt, cFileNameOutputEigen, X->Bind.Def.CDataFileHead, X->Bind.Def.k_exct-1, myrank);
    if(childfopenALL(sdt, "wb", &fp)!=0){
      fclose(fp);
      exitMPI(-1);
      }
    fwrite(&X->Bind.Check.idim_max, sizeof(long int), 1, fp);
    fwrite(v1, sizeof(complex double),X->Bind.Check.idim_max+1, fp);
    fclose(fp);
  }

  return 0;
}