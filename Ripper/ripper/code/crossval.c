#include <stdio.h>
#include "ripper.h"
#include "mdb.h"

static double std_err(double *,double,int);

/*****************************************************************************/

typedef void (*training_f)(vec_t *);
typedef double (*test_f)(vec_t *);

void cross_validate(int folds,vec_t *train_data,training_f train,test_f test)
{
    int m,i;
    double *errors,err,total_err;
    double *times,tm,total_tm;
    DATA *data1,*data2;
    FILE *out_fp;

    if (folds==0) /* leave one out flag */ {
	folds = vmax(train_data);
    }
    m = vmax(train_data)/folds;
    trace(SUMM) { 
	printf("// will use %d of %d examples for testing\n",
	       m,vmax(train_data));
	fflush(stdout);
    }
    errors = newmem(folds,double);
    times = newmem(folds,double);
    total_err = total_tm = 0.0;
    data1 = new_data(vmax(train_data)-m);
    data2 = new_data(m);
    stratify_and_shuffle_data(train_data,folds);
    for (i=0; i<folds; i++) {
	/* partition the data into training and testing sets */
	ith_stratified_partition(train_data,i,folds,data1,data2);
	printf("-------------------------");
	printf(" run %2d ",i+1);
	printf("-------------------------\n");
	start_clock(); 
	trace(SUMM) {
	    printf("// timing training %d...\n",i+1);
	    fflush(stdout);
	}
	(*train)(data1);
	tm = elapsed_time(); 
	trace(SUMM) {
	    printf("// training took %.2f sec\n",tm);
	    fflush(stdout);
	}
	err = 100*(*test)(data2);
	printf("Error rate on holdout data is %g%%\n",err);
	printf("Running average of error rate is %g%%\n",
	       (total_err+err)/(i+1));
	times[i] = tm;
	total_tm += tm;
	errors[i] = err;
	total_err += err;
    } /*for each fold */
    printf("============================");
    printf(" statistical summary ");
    printf("============================\n");
    printf("Average error: %.2f%% +/- %.2f%%     <<\n",
	   total_err/folds,
	   std_err(errors,total_err,folds));
    printf("Average time:  %.2f  +/- %.2f\n",
	   total_tm/folds,
	   std_err(times,total_tm,folds));
    freemem(errors);
    freemem(times);
}

static double std_err(v,tot,n)
double *v, tot;
int n;
{
    int i;
    double variance_sum,sd,correction,sd_plus;

    if (n==1) return 0;

    variance_sum=0;
    for (i=0; i<n; i++) variance_sum += (v[i]-tot/n)*(v[i]-tot/n);
    sd = sqrt(variance_sum/n);
    correction = ((double)n)/((double)n-1);
    sd_plus = sd*correction;
    return sd_plus / sqrt((double) n);
}


