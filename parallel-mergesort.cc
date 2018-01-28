/**
 *  \file parallel-mergesort.cc
 *
 *  \brief Implement your parallel mergesort in this file.
 */

#include <omp.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sort.hh"

void seqMerge(keytype* arr,int start,int middle,int end)
{
	int position_a=0,position_b=0;
	int len_a = middle-start;
	int len_b = end-middle+1;
	int position_arr = start;


	//Copy first part in A and second part in B

	keytype* A = newKeys(len_a);
	memcpy(A,arr,len_a*sizeof(keytype));

	keytype* B = newKeys(len_b);
	memcpy(B,arr + middle,len_b*sizeof(keytype));

	for(int i=0;i<len_a;i++) A[i] = arr[i+start];

		for(int i=0;i<len_b;i++) B[i] = arr[i+middle];


			while(position_a < len_a && position_b < len_b)
			{
				if(A[position_a] < B[position_b])
				{
					arr[position_arr] = A[position_a];
					position_a ++ ;
					position_arr++;
				}
				else
				{
					arr[position_arr] = B[position_b];
					position_b ++;
					position_arr++;

				}
			}
			if(position_a < len_a)
			{
				for(int i = position_a;i<len_a;i++)
				{
					arr[position_arr] = A[i];
					position_arr ++;
				}
			}
			else if(position_b < len_b)
			{
				for(int i = position_b;i<len_b;i++)
				{
					arr[position_arr] = B[i];
					position_arr++;

				}
			}



}

void seqMergeSort(keytype* arr, int start,int end)
{
	if(start>=end) return;
	int mid = int((start+end+1)/2);
	seqMergeSort(arr,start,mid-1);
	seqMergeSort(arr,mid,end); //here mid is the beginning of the second array
	seqMerge(arr,start,mid,end);
	return;

}

int binary_search(keytype* arr,int val,int len_arr)
{
  //Should return index of array such that arr[index] <= val
  
  int start = 0;
  int end = len_arr-1;
  int mid = (start+end)/2;

  while(start<=end)
  {
      mid = (start+end)/2;
      if(arr[mid] == val)
      {
        return mid;
      }
      else if(arr[mid] > val)
      {
          end = mid-1;
      }
      else 
      {
          start = mid +1;
      }

  }


  if(start >= len_arr) return start; //Greater than all elements in array(=len_arr)
  else if(end <0) return end; //Lesser than all elements in array - (-1)
  else return ((start+end)/2); //between 2 values

}

void parallel_merge(keytype* arr,int start,int middle,int end)
{
  //middle is whether the second array starts
  //Initialize vectors A and B and copy elements
  

  if(start>= middle || middle>end || start>=end) 
  {
    
    return;
  } 
  if(end-start==1)
  {
    //This case is when there are only 2 elements to be merged
    if(arr[start] > arr[end])
    {
      int temp = arr[start];
      arr[start] = arr[end];
      arr[end]= temp;

    }
    return;
  }
  #pragma omp task
  int position_a=0,position_b=0;
  int len_a = middle-start;
  int len_b = end-middle+1;
  int position_arr = start;
  #pragma omp taskwait
  //Copy first part in A and second part in B
  keytype* A = newKeys(len_a);
  keytype* B = newKeys(len_b);

  #pragma omp task
  memcpy(A,arr+start,len_a*sizeof(keytype));

  memcpy(B,arr + middle,len_b*sizeof(keytype));
  
  int mid_a = len_a/2;
  #pragma omp taskwait

  keytype v = A[mid_a];
  int k = binary_search(B,v,len_b);

  if(k>=0 && k<len_b)
  {
    int p = start+mid_a;
    int q = p+k+1;
    int r = q+len_a-mid_a;

    //Below 3 can be parallelized
    //
    int i=0;
    #pragma omp parallel shared(arr,B,p,q,r,mid_a,end)private(i)
    {
	
    	#pragma omp for     	
	for(i=p;i<q;i++)
    	{
      		arr[i] = B[i-p];

    	}
    	#pragma omp for 
	for(i=q;i<r;i++)
    	{
      		arr[i] = A[mid_a+i-q];
    	}
    	#pragma omp for 
	for(i=r;i<=end;i++)
    	{
      		arr[i] = B[k+1+i-r];
    	}
    }

    #pragma omp task
    parallel_merge(arr,start,p,q-1);
    parallel_merge(arr,q,r,end);
    #pragma omp taskwait

  }
  else if(k<0)
  {
    //all elements in B are greater than mid-point of A
    //a1b1a2b2, there is no b1, as all elements are greater than mid-point of a, so there is just b2, which includes all of b
    //so we can directly call parallel merge , no need to modify elements , just modify the call(just a1b1b2)
   parallel_merge(arr,start+mid_a+1,middle,end); //mid_a + 1 as have to merge a2b, mid_a is in a1

  }
  else if(k>=len_b)
  {
      //all elements of b are less than mid_point of A
      //a1b1a2b2, no b2, therefore just a1,b1a2
      int p = start+mid_a;
      int q = p+len_b;//here I am assuming that k= len_b
      //int r = q+len_a-mid_a;
      int i=0;
 //     #pragma omp for shared(arr,B,p,q)private(i)
      for(i=p;i<q;i++)
      {
        arr[i] = B[i-p];
      }
      i=0;
//      #pragma omp for shared(arr,A,q,end,mid_a)private(i)
      for(i=q;i<=end;i++)
      {
        arr[i] = A[mid_a+i-q];
      }
      //just a1,b1 has to be merged, a2 can be left as it is
      parallel_merge(arr,start,p,q-1);

  }
 
}

void par_sort(keytype* arr,int start,int end)
{
        if(start>=end) return;
        int mid = int((start+end+1)/2);
        #pragma omp task
        par_sort(arr,start,mid-1);
        par_sort(arr,mid,end);
        #pragma omp taskwait
        parallel_merge(arr,start,mid,end);


}

void parallelSort (int N, keytype* A)
{
	int sp = 1; //0-sequential 1-parallel
	if(sp == 0)
	{
		omp_set_num_threads(8);
		#pragma omp parallel
		{
			printf("world " );
		}
		printf("\n");
	  /* Lucky you, you get to start from scratch */
		seqMergeSort(A,0,N-1);	
	}
	else
	{
		omp_set_num_threads(8);
		#pragma omp parallel
		{
			#pragma omp single nowait
			par_sort(A,0,N-1);
		}
		
	}			
}


/* eof */
