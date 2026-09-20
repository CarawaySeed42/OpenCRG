#include "mex.h"
#include <stdio.h>
#include <string.h>
#include "crgBaseLib.h"
#include "crgBaseLibPrivate.h"

/* Persistent state */
static int datasetId = -1;
static int isInitialized = 0;

/* Cleanup function */
//void cleanup(void)
//{
//    if (datasetId != -1) {
//        mexPrintf("Cleaning up dataset %d\n", datasetId);
//        /* crgDataSetRelease(datasetId); */
//        datasetId = -1;
//    }
//    mexUnlock();
//}

/*int list_count = getNoDatasets();
CrgDataStruct** list = getDataSetList();*/

/* Gateway function */
void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]){
    int datasetId;
    char cmd[16];

    /* One-time initialization */
    //if (!isInitialized) {
    //    mexLock();                 /* prevent unloading */
    //    mexAtExit(cleanup);        /* register cleanup */
    //    isInitialized = 1;
    //}

    /* Check command */
    if (nrhs < 1 || !mxIsChar(prhs[0])) {
        mexErrMsgTxt("First argument must be a command string.");
    }

    mxGetString(prhs[0], cmd, sizeof(cmd));

    /* Dispatch */
    if (_strcmpi(cmd, "open") == 0) { /* Open CRG */

        if (nrhs != 2) {
            mexErrMsgIdAndTxt("crgMex:nrhs", "Two inputs required: command, filename.");
        }

        if (nlhs < 1) {
            mexErrMsgIdAndTxt("crgMex:nlhs", "One output required.");
        }

        if (!mxIsChar(prhs[1])) {
            mexErrMsgIdAndTxt("crgMex:inputType", "Input must be a character array.");
        }

        char* filename = mxArrayToString(prhs[1]);
        if (filename == NULL) {
            mexErrMsgIdAndTxt("crgMex:conversionError", "Failed to convert input to string.");
        }

        datasetId = crgLoaderReadFile(filename);
        mxFree(filename);

        plhs[0] = mxCreateNumericMatrix(1, 1, mxINT32_CLASS, mxREAL);
        *((int32_T*)mxGetData(plhs[0])) = datasetId;
    }
    else if (_strcmpi(cmd, "close") == 0) { /* Close CRG */

        if (nrhs != 2) {
            mexErrMsgIdAndTxt("crgMex:nrhs", "Two inputs required: command, dataset id.");
        }

        int datasetId = (int)mxGetScalar(prhs[1]);
        int close_success = crgDataSetRelease(datasetId);

        if (nlhs > 0) {
            plhs[0] = mxCreateLogicalScalar((bool)close_success);
        }
    }
    else if (_strcmpi(cmd, "eval_uv2z") == 0) {  /* Evaluate Z at (u,v) */

        if (nrhs != 3) {
            mexErrMsgIdAndTxt("crgMex:nrhs",
                "Three inputs required: command, dataset id, puv.");
        }

        if (nlhs != 1) {
            mexErrMsgIdAndTxt("crgMex:nlhs",
                "One output required.");
        }

        /* Dataset ID */
        int dataSetId = (int)mxGetScalar(prhs[1]);

        /* PUV array */
        if (!mxIsDouble(prhs[2]) || mxIsComplex(prhs[2]) ||
            mxGetN(prhs[2]) != 2) {
            mexErrMsgIdAndTxt("crgMex:inputType",
                "PUV must be an (N x 2) real double matrix.");
        }

        mwSize np = mxGetM(prhs[2]);
        double* puv = mxGetPr(prhs[2]);

        /* Split u and v (column-major layout) */
        double* u = puv;           /* first column */
        double* v = puv + np;      /* second column */

        /* Output Z */
        plhs[0] = mxCreateDoubleMatrix(np, 1, mxREAL);
        double* z = mxGetPr(plhs[0]);

        /* Get CRG data and evaluate uv to z */
        CrgDataStruct* crgData = crgDataSetAccess(dataSetId);

        if (NULL == crgData) {
            mexErrMsgIdAndTxt("crgMex:dataAccess", "CRG data access failed (dataSetId %d).", dataSetId);
        }

        CrgOptionsStruct* crgOptions = &(crgData->options);

        /* Evaluate */
        for (int i = 0; i < np; ++i)
        {
            double cu = u[i];
            double cv = v[i];
            crgDataEvaluv2z(crgData, crgOptions, cu, cv, &z[i]);
        }
    }
    else if (_strcmpi(cmd, "eval_uv2xy") == 0) {  /* Evaluate X/Y at (u,v) */

        if (nrhs != 3) {
            mexErrMsgIdAndTxt("crgMex:nrhs",
                "Three inputs required: command, dataset id, puv.");
        }

        if (nlhs != 1) {
            mexErrMsgIdAndTxt("crgMex:nlhs",
                "One output required.");
        }

        /* Dataset ID */
        int dataSetId = (int)mxGetScalar(prhs[1]);

        /* PUV array */
        if (!mxIsDouble(prhs[2]) || mxIsComplex(prhs[2]) ||
            mxGetN(prhs[2]) != 2) {
            mexErrMsgIdAndTxt("crgMex:inputType",
                "PUV must be an (N x 2) real double matrix.");
        }

        mwSize np = mxGetM(prhs[2]);
        double* puv = mxGetPr(prhs[2]);

        /* Split u and v (column-major layout) */
        double* u = puv;           /* first column */
        double* v = puv + np;      /* second column */

        /* Output PXY (N x 2) */
        plhs[0] = mxCreateDoubleMatrix(np, 2, mxREAL);
        double* pxy = mxGetPr(plhs[0]);

        /* Column pointers */
        double* x = pxy;           /* first column */
        double* y = pxy + np;      /* second column */

        /* Get CRG data and evaluate uv to xy */
        CrgDataStruct* crgData = crgDataSetAccess(dataSetId);

        if (NULL == crgData) {
            mexErrMsgIdAndTxt("crgMex:dataAccess",
                "CRG data access failed (dataSetId %d).", dataSetId);
        }

        CrgOptionsStruct* crgOptions = &(crgData->options);

        /* Evaluate */
        for (mwSize i = 0; i < np; ++i) {
            crgDataEvaluv2xy(crgData, crgOptions, u[i], v[i], &x[i], &y[i]);
        }
    }
    else if (_strcmpi(cmd, "eval_xy2uv") == 0) {  /* Evaluate U/V at (x,y) */

        if (nrhs != 3) {
            mexErrMsgIdAndTxt("crgMex:nrhs",
                "Three inputs required: command, dataset id, pxy.");
        }

        if (nlhs != 1) {
            mexErrMsgIdAndTxt("crgMex:nlhs",
                "One output required.");
        }

        /* Dataset ID */
        int dataSetId = (int)mxGetScalar(prhs[1]);

        /* PXY array */
        if (!mxIsDouble(prhs[2]) || mxIsComplex(prhs[2]) ||
            mxGetN(prhs[2]) != 2) {
            mexErrMsgIdAndTxt("crgMex:inputType",
                "PXY must be an (N x 2) real double matrix.");
        }

        mwSize np = mxGetM(prhs[2]);
        double* pxy = mxGetPr(prhs[2]);

        /* Split x and y (column-major layout) */
        double* x = pxy;           /* first column */
        double* y = pxy + np;      /* second column */

        /* Output PUV (N x 2) */
        plhs[0] = mxCreateDoubleMatrix(np, 2, mxREAL);
        double* puv = mxGetPr(plhs[0]);

        double* u = puv;           /* first column */
        double* v = puv + np;      /* second column */

        /* Get CRG data */
        CrgDataStruct* crgData = crgDataSetAccess(dataSetId);

        if (NULL == crgData) {
            mexErrMsgIdAndTxt("crgMex:dataAccess",
                "CRG data access failed (dataSetId %d).", dataSetId);
        }

        int cpId = crgContactPointCreate(dataSetId);
        CrgContactPointStruct* cp = crgContactPointGetFromId(cpId);

        /* Evaluate */
        for (int i = 0; i < np; ++i) {

            crgEvalxy2uvPtr(cp, x[i], y[i], &u[i], &v[i]);

                //if (rc != 0) {
                //    mexErrMsgIdAndTxt("crgMex:evalFailed",
                //        "crgDataEvalxy2uv failed at index %d (error %d).",
                //        (int)i, rc);
                //}
        }

        crgContactPointDelete(cpId);
    }
    else {
        mexErrMsgTxt("Unknown command.");
    }
}
