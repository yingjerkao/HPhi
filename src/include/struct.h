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
/*-------------------------------------------------------------
 *[ver.2009.3.31]
 * Exact Diagonalization with Lanczos Method
 *-------------------------------------------------------------
 * Copyright (C) 2006- Takahiro MISAWA. All rights reserved.
 *-------------------------------------------------------------*/

#ifndef HPHI_STRUCT_H
#define HPHI_STRUCT_H

/*=================================================================================================*/
struct DefineList{
  char *CDataFileHead;				/**< Read from Calcmod in readdef. Header of output file such as Green's function */
  char  *CParaFileHead;				/**< Read from Calcmod in readdef. It is not used. Just for the compatibility to mVMC */
  int nvec;				 	/**< Read from Calcmod in readdef */
  int k_exct;				  	/**< Read from Calcmod in readdef */
  int LanczosEps; 				/**< log(10 base) of the convergence threshold. Read from Calcmod in readdef */
  int  LanczosTarget;				/**< Which eigenstate is used to check convergence. Read from Calcmod in readdef. */
  int read_hacker; 				
  int  READ;    /**< */
  int  WRITE;    /**< */
  long int global_off;  /**< */
  
  int  Nsite;    /**< */
  int  NsiteMPI;    /**< */
  int  Nup;    /**< Read from modpara in readdef */
  int Ndown;    /**< */
  int  Total2Sz;    /**< */
  int  Total2SzMPI;    /**< */
  int  Ne;    /**< */
  int  Nsize;    /**< */
  int Lanczos_max;    /**< */
  long int initial_iv;    /**< */
  
  int istep;    /**< */
  int irand;    /**< */
  int St;    /**< */
  
  int *LocSpn;    /**< [NLocSpn] */
  int  NLocSpn;    /**< */
  int NCond;
  int iFlgGeneralSpin;
  int iFlgSzConserved;
  
  int fidx;    /**< */
  long int *Tpow;    /**< [2 * Nsite] 2^n */
  long int *OrgTpow;    /**< [2 * Nsite] 2^n */
  long int *SiteToBit; /**< [Nsite] */
  
  int *EDChemi;    /**< [Nsite] */
  int  EDNChemi;   /**< */  
  int  *EDSpinChemi;  /**< [Nsite] */

  double   *EDParaChemi;  /**< [Nsite] */

  //[s] Transfer
  int  NTransfer;				/**< Number of transfer integrals obtained by a def file.*/
  int  EDNTransfer;				/**< Number of transfer integrals for calculation. */
  int  **Transfer;				/**< Index of transfer integrals \f$t_{ij\sigma_i\sigma_j}\f$ obtained by a def file. \n
						   Data Format [Ntransfer][3]\n
						   Ntransfer-> a parameter defined by struct.h.\n
						   0=\f$i\f$, 1=\f$j\f$, 2=\f$ \sigma_i=\sigma_j\f$. */
  int  **EDTransfer;				/**< Index of transfer integrals for calculation. \n
						   Data Format [Ntransfer][3]\n
						   Ntransfer-> a parameter defined by struct.h.\n
						   0=\f$i\f$, 1=\f$j\f$, 2=\f$ \sigma_i=\sigma_j\f$. */
  int  **GeneralTransfer;			/**< Index of transfer integrals obtained by a def file. \n
						   Data Format [Ntransfer][4]: 0->site number i, 1-> spin index on i, 2-> site number j, 3-> spin index on j. */
  int  **EDGeneralTransfer;			/**< Index of transfer integrals for calculation. \n
						   Data Format [Ntransfer][4]: 0->site number i, 1-> spin index on i, 2-> site number j, 3-> spin index on j. */
  double *ParaTransfer;				/**< Value of transfer integrals by a def file. \n
						   Data Format [Ntransfer]. */
  double complex *ParaGeneralTransfer;		/**< Value of general transfer integrals  by a def file. \n
						   Data Format [Ntransfer]. */
  double *EDParaTransfer;    			/**< Value of general transfer integrals for calculation. \n
						   Data Format [Ntransfer]. */
  double complex *EDParaGeneralTransfer;	/**< Value of general transfer integrals  by a def file. \n
						   Data Format [Ntransfer]. */
  //[e] Transfer
  
  int   **CoulombIntra;  /**< [NCoulombIntra][1] */
  int NCoulombIntra;  /**< */
  double  *ParaCoulombIntra;  /**< [NCoulombIntra] */

  int   **CoulombInter;  /**< [NCoulombInter][2] */
  int NCoulombInter;    /**< */
  double  *ParaCoulombInter; /**< [NCoulombInter] */
 
  int  **HundCoupling;  /**< */
  int  NHundCoupling;  /**< */
  double   *ParaHundCoupling;  /**< */

  int   **PairHopping;  /**< */
  int  NPairHopping;    /**< */
  double   *ParaPairHopping;/**< */
  
  int   **ExchangeCoupling;  /**< */
  int    NExchangeCoupling;    /**< */
  double *ParaExchangeCoupling;  /**< */

  int    NIsingCoupling;    /**< */
  
  int   **PairLiftCoupling;  /**< */
  int    NPairLiftCoupling;    /**< */
  double *ParaPairLiftCoupling;  /**< */


  //[s] For InterAll
  int **InterAll;  /**< [NinterAll][8] Interacted quartet */
  int **InterAll_OffDiagonal;  /**< [NinterAll_OffDiagonal][8] Interacted quartet */
  int **InterAll_Diagonal;  /**< [NinterAll_Diagonal][4] Interacted quartet */
  int NInterAll;    /**< */
  int NInterAll_Diagonal;    /**< */
  int NInterAll_OffDiagonal;    /**< */
  double complex *ParaInterAll;  /**< */
  double *ParaInterAll_Diagonal;
  double complex *ParaInterAll_OffDiagonal;  
  //[e] For InterAll

  int **CisAjt;  /**< */
  int NCisAjt;    /**< */


  int   **CisAjtCkuAlvDC; /**< */
  int NCisAjtCkuAlvDC; /**< */
	
  int iCalcType;
  /**< An integer for selecting calculation type. 0:Lanczos, 1:TPQCalc, 2:FullDiag.*/

  int iCalcEigenVec;
  /**< An integer for selecting method to calculate eigenvectors. 0:Lanczos+CG, 1: Lanczos. default value is set as 0 in readdef.c*/  

  int iInitialVecType;
  /**< An integer for setting a type of inital vectors. 0:complex type, 1: real type. default value is set as 0 in readdef.c*/  
  
  int iFlgFiniteTemperature;    /**< */
  int iCalcModel;
  /**<  An integer for selecting calculation model. 0:Hubbard, 1:Spin, 2:Kondo, 3:HubbardGC, 4:SpinGC, 5:KondoGC, 6:HubbardNConserved*/
  int iOutputMode;
  /**<   An integer for selecting output mode. 0: OneBodyG and TwoBodyG. 1: OneBodyG and TwoBodyG and correlations for charge and spin.*/

  /**< An integer for selecting output an eigenvector. 0: no output, 1:output.*/
  int iOutputEigenVec;

    /**< An integer for selecting output an eigenvector. 0: no input, 1:input*/
  int iInputEigenVec;

    /**< An integer for selecting output a Hamiltonian. 0: no output, 1:output*/
    int iOutputHam;

};

struct CheckList{
  unsigned long int  idim_max; /**< */
  unsigned long int  idim_maxMPI; /**< */
  unsigned long int     sdim;    /**< */
  double   max_mem;  /**< */

};

struct LargeList{
  double complex prdct;  /**< */
  int itr;  /**< */
  long int iv;
  long int  i_max;
  double complex tmp_trans;
  double  complex  tmp_J;
  
  long unsigned int is1_up;
  long unsigned int is1_down;
  long unsigned int is2_up;
  long unsigned int is2_down;
  
  
  int mode;
  double sgn;
  long unsigned int  is1_spin;
  long unsigned int  is2_spin;
  long unsigned int  is3_spin;
  long unsigned int  is4_spin;
  int isite1;
  int isite2;
  int isite3;
  int isite4;

  long unsigned int A_spin;
  long unsigned int B_spin;
  long unsigned int irght;
  long unsigned int     ilft;
  long unsigned int     ihfbit;
  long unsigned int isA_spin;
  long unsigned int  isB_spin;
  double complex      tmp_V;
};

struct PhysList{
  //double energy,doublon;
  double energy;
  double doublon;
  double num;
  /*[s] For TPQ*/
  double var;
  /*[e] For TPQ*/

  /*[s] For Full Diagonalization*/
  int eigen_num;  
  double num_up;
  double num_down;
  double s2;
  double sz;
  double *all_energy;
  double *all_doublon;
  double *all_sz;
  double *all_s2;
  double *all_num_up;
  double *all_num_down; 
  /*[e] For Full Diagonalization*/
  
  double *spin_real_cor;
  double *charge_real_cor;
  double *loc_spin_z;
  double Target_energy;
};	

//For Boost
struct BoostList {
  int flgBoost;
  long unsigned int R0, W0, num_pivot, ishift_nspin;
  int NumarrayJ;
  double complex ***arrayJ;
  double complex vecB[3];
  int  **list_6spin_star;
  int ***list_6spin_pair;
};

/*=================================================================================================*/
struct BindStruct{
  struct DefineList  Def;
  struct CheckList   Check;
  struct LargeList   Large;
  struct PhysList    Phys;
  struct BoostList   Boost;
};
/*=================================================================================================*/
struct EDMainCalStruct{
  struct BindStruct Bind;
};

struct TimeKeepStruct{
  time_t tstart;
  time_t tnow;
  time_t tend;
};

/*global variables---------------------------------------------*/
struct EDMainCalStruct X;
/*-------------------------------------------------------------*/

#endif /* HPHI_STRUCT_H */
