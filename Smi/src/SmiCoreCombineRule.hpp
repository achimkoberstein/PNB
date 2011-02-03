// Copyright (C) 2004, International Business Machines
// Corporation and others.  All Rights Reserved.
//
// SmiCoreCombineRule.hpp: interface for the SmiCoreCombineRule classes.
//
//
// Alan King
// 9 March 2004
//////////////////////////////////////////////////////////////////////

#if !defined(SmiCoreCombineRule_HPP)
#define SmiCoreCombineRule_HPP

#include "CoinPragma.hpp"
#include "CoinPackedVector.hpp"
#include <string>
//#include <vector>
using namespace std;

/** This deals with combining Core and Stochastic data.

	In the Stochastic MPS standard, stochastic data updates
	the underlying core lp data.  To specify a new scenario,
	one only has to identify those data that are different.
	So, in a sense, the stochastic data is really a "diff" between
	the scenario and the core data. This class specifies how
	to perform the "undiff", that is, how to combine core
	and stochastic data.

	And of course, a complete implementation specifies the
	"diff" part as well.  Now during a fit of original confusion
	in the birth of the SMPS standard, we decided to make
	default combine rule "replace", which has a rather special
	"diff", but we've learned to live with it.

	There only needs to be one of these classes. so they're
	singletons.
	*/
class SmiCoreCombineRule
{
public:
  /**@name Virtual Functions: Process and Diff */
  //@{
  /// Process
    /** Combines the values in d1 with the values in cpv2, where o1 is the offset to apply to d1.
    @param d1 the values to be modified
    @param o1 the offset to be applied to d1
    @param cpv2 the vector that contains the values are added to / replace the values in d1
    */
    virtual void Process(double *d1, int o1, const CoinPackedVector &cpv2, char *type=0)=0;

    /** Combines the values in d1 with the values in dels. dels contains len elements with indices inds. o1 is the offset to apply to d1.
    @param d1 the values to be modified
    @param o1 the offset to be applied to d1
    @param len the length of the inds and dels array
    @param inds the indices for the values in dels
    @param dels the values that are added to / replace the values in d1
    */
    virtual void Process(double *d1, int o1, const int len, const int* inds, const double *dels, char *type=0)=0;

	/** Combines the values cpv1 with the values in cpv2 by adding or replacing them and returns a newly created CPV.
    @param cpv1 the first vector
    @param cpv2 the second vector
    @return a new CPV that contains the combined data
    */
    virtual CoinPackedVector * Process(CoinPackedVector *cpv1, CoinPackedVector *cpv2, char *type=0)=0;

    /** Combines the values from dr and cpv into the array dels. The dr_len entries in dr are copied into
        dels and if there are corresponding entries in cpv, the values are added to / replace the values in dels.
    @param dr the values that are combined with the values in cpv and copied into dels
    @param dr_len the length of the array dr
    @param cpv the vector that contains the values are added to / replace the updated values in dels
    @param dels the array that contains the newly combined data
    @param indx the indices array for dels
    */
    virtual int Process(double *dr,const int dr_len,CoinPackedVector *cpv,double *dels,int *indx)=0;

    /** Combines the values from dr and cpv_nels into the array dels. The dr_len entries in dr are copied into
        dels and if there are corresponding entries in cpv_ind, the values in cpv_els are added to / replace the values in dels.
    @param dr the values that are combined with the values in cpv and copied into dels
    @param dr_len the length of the array dr
    @param cpv_nels the number of elements that are added to / replace the updated values in dels
    @param cpv_ind the indices for the values that are added to / replace the updated values in dels
    @param cpv_els the values that are added to / replace the updated values in dels
    @param dels the array that contains the newly combined data
    @param indx the indices array for dels
    */
    virtual int Process(double *dr,const int dr_len,const int cpv_nels,const int* cpv_ind,const double *cpv_els,double *dels,int *indx)=0;
    virtual ~SmiCoreCombineRule(){}
};

//////////////////////////////////////////////////////////////////////
// SmiCoreCombineReplace
// -- concrete "replace" rule -- singleton pattern
//////////////////////////////////////////////////////////////////////
class SmiCoreCombineReplace : public SmiCoreCombineRule
{
public:
    /// Return the instance of this combine rule
	static SmiCoreCombineReplace * Instance();

	/// Delete the instance of this combine rule
	static void ClearInstance();

    /** Replaces the values in d1 with the values in cpv2, where o1 is the offset to apply to d1.
    @param d1 the values to be modified
    @param o1 the offset to be applied to d1
    @param cpv2 the vector that contains the values that replace the values in d1
    */
	virtual void Process(double *d1, int o1, const CoinPackedVector &cpv2, char *type=0);

	/** Replaces the values in d1 with the values in dels. dels contains len elements with indices inds. o1 is the offset to apply to d1.
    @param d1 the values to be modified
    @param o1 the offset to be applied to d1
    @param len the length of the inds and dels array
    @param inds the indices for the values in dels
    @param dels the values that replace the values in d1
    */
	virtual void Process(double *d1, int o1, const int len, const int* inds, const double *dels, char *type=0);

	/** Combines the values cpv1 with the values in cpv2 by replacing the values in the first vector by the corresponding values of the second vector.
	    The result is a newly created CPV which is returned.
    @param cpv1 the first vector
    @param cpv2 the second vector
    @return a new CPV that contains the combined data
    */
	virtual CoinPackedVector * Process(CoinPackedVector *cpv1, CoinPackedVector *cpv2, char *type=0);

	/** Combines the values from dr and cpv into the array dels. The dr_len entries in dr are copied into
        dels and if there are corresponding entries in cpv, the values replace the values in dels.
    @param dr the values that are combined with the values in cpv and copied into dels
    @param dr_len the length of the array dr
    @param cpv the vector that contains the values that replace the updated values in dels
    @param dels the array that contains the newly combined data
    @param indx the indices array for dels
    */
	virtual int Process(double *dr,const int dr_len,CoinPackedVector *cpv,double *dels,int *indx);

	/** Combines the values from dr and cpv_nels into the array dels. The dr_len entries in dr are copied into
        dels and if there are corresponding entries in cpv_ind, the values in cpv_els replace the values in dels.
    @param dr the values that are combined with the values in cpv and copied into dels
    @param dr_len the length of the array dr
    @param cpv_nels the number of elements that replace the updated values in dels
    @param cpv_ind the indices for the values that replace the updated values in dels
    @param cpv_els the values that replace the updated values in dels
    @param dels the array that contains the newly combined data
    @param indx the indices array for dels
    */
	virtual int Process(double *dr,const int dr_len,const int nels, const int* cpv_ind,const double *cpv_els,double *dels,int *indx);
protected:
	SmiCoreCombineReplace(){}
private:
    /// A pointer to the only instance of this class
	static SmiCoreCombineReplace * _instance;
};

//////////////////////////////////////////////////////////////////////
// SmiCoreCombineAdd
// -- concrete "add" rule -- singleton pattern
//////////////////////////////////////////////////////////////////////
class SmiCoreCombineAdd : public SmiCoreCombineRule
{
public:
    /// Return the instance of this combine rule
	static SmiCoreCombineAdd * Instance();
	
	/// Delete the instance of this combine rule
	static void ClearInstance();

	/** Add to the values in d1 the values in cpv2, where o1 is the offset to apply to d1.
    @param d1 the values to be modified
    @param o1 the offset to be applied to d1
    @param cpv2 the vector that contains the values that are added to the values in d1
    */
	virtual void Process(double *d1, int o1, const CoinPackedVector &cpv2, char* type=0);

    /** Add to the values in d1 the values in dels. dels contains len elements with indices inds. o1 is the offset to apply to d1.
    @param d1 the values to be modified
    @param o1 the offset to be applied to d1
    @param len the length of the inds and dels array
    @param inds the indices for the values in dels
    @param dels the values that are added to the values in d1
    */
	virtual void Process(double *d1, int o1, const int len, const int* inds, const double *dels, char *type=0);

	/** Combines the values cpv1 with the values in cpv2 by adding them and returns a newly created CPV.
    @param cpv1 the first vector
    @param cpv2 the second vector
    @return a new CPV that contains the combined data
    */
	virtual CoinPackedVector * Process(CoinPackedVector *cpv1, CoinPackedVector *cpv2, char* type=0);

	/** Combines the values from dr and cpv into the array dels. The dr_len entries in dr are copied into
        dels and if there are corresponding entries in cpv, the values are added to the values in dels.
    @param dr the values that are combined with the values in cpv and copied into dels
    @param dr_len the length of the array dr
    @param cpv the vector that contains the values are added to the updated values in dels
    @param dels the array that contains the newly combined data
    @param indx the indices array for dels
    */
	virtual int Process(double *dr,const int dr_len,CoinPackedVector *cpv,double *dels,int *indx);

	/** Combines the values from dr and cpv_nels into the array dels. The dr_len entries in dr are copied into
        dels and if there are corresponding entries in cpv_ind, the values in cpv_els are added to the values in dels.
    @param dr the values that are combined with the values in cpv and copied into dels
    @param dr_len the length of the array dr
    @param cpv_nels the number of elements that are added to the updated values in dels
    @param cpv_ind the indices for the values that are added to the updated values in dels
    @param cpv_els the values that are added to the updated values in dels
    @param dels the array that contains the newly combined data
    @param indx the indices array for dels
    */
	virtual int Process(double *dr,const int dr_len,const int nels, const int* cpv_ind,const double *cpv_els,double *dels,int *indx);
protected:
	SmiCoreCombineAdd(){}
private:
    /// A pointer to the only instance of this class
	static SmiCoreCombineAdd * _instance;
};


#endif // !defined(SmiCoreCombineRule_HPP)
