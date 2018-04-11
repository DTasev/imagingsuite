//<LICENSE>

#include <vector>
#include <sstream>
#include <string>
#include <iostream>
#include <emmintrin.h>
#include <cmath>
#include <math.h>

#include "fftw3.h"

#include <base/timage.h>
#include <base/tpermuteimage.h>
#include <math/mathconstants.h>
#include <interactors/interactionbase.h>
#include <ReconException.h>

#include "fdkbp.h"

#ifndef DEGTORAD
static const double DEGTORAD = dPi/ 180.0;
#endif

#ifndef MARGIN
static const unsigned int MARGIN = 5;
#else
#error "MARGIN IS DEFINED"
#endif


// Matrix element m[i,j] for matrix with c columns
#define m_idx(m1,c,i,j) m1[i*c+j]


FDKbp::FDKbp(kipl::interactors::InteractionBase *interactor) :
    FdkReconBase("muhrec","FDKbp",BackProjectorModuleBase::MatrixXYZ,interactor)
{


}

FDKbp::~FDKbp()
{


}

int FDKbp::Initialize()
{
    return 0;
}

int FDKbp::InitializeBuffers(int width, int height)
{
    prepareFFT(width,height);
    return 0;
}

int FDKbp::FinalizeBuffers()
{
    cleanupFFT();

    return 0;
}

void FDKbp::multiplyMatrix(double *mat1, double *mat2, double *result, int rows, int columns, int columns1){

    int i,j,k;

    for (i = 0; i < rows; i++) {
         for (j = 0; j < columns; j++) {
             float acc = 0.0; // why not double?
             for (k = 0; k < columns1; k++) {
                 acc += m_idx(mat1,columns1,i,k) * m_idx(mat2,columns,k,j);
             }
             m_idx(result,columns,i,j) = acc;
         }
    }

}

void FDKbp::getProjMatrix(float angles, double *nrm, double *proj_matrix){

    // compute geometry matrix: detector oriented as the z axis (in case of no tilt), source perpendicular to x

    double extrinsic[16] =   {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}; // Extrinsic matrix
    double intrinsic[12] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}; // Intrinsic matrix
    double sad = mConfig.ProjectionInfo.fSOD; ; // Distance: source to axis = source to object distance
    double sid = mConfig.ProjectionInfo.fSDD; // Distance: source to image = source to detector distance


   double offCenter =  (mConfig.ProjectionInfo.fpPoint[0]-mConfig.ProjectionInfo.roi[0]-mConfig.ProjectionInfo.fCenter)*mConfig.MatrixInfo.fVoxelSize[0]; // in world coordinate
   // check here projection_roi or roi, it is actually the same

   // --------------- added -roi[0] because i have removed from fdkreconbase


   double cam[3] = {0.0, 0.0, 0.0}; // Location of Camera

   if (mConfig.ProjectionInfo.eDirection == kipl::base::RotationDirCW) {
       cam[0] = sad*cos(-angles*dPi/180);
       cam[1] = sad*sin(-angles*dPi/180);
   }
   else{
       cam[0] = sad*cos(angles*dPi/180);
       cam[1] = sad*sin(angles*dPi/180);
   }
    double plt[3] = {0.0,0.0,0.0}; // Detector orientation  Panel left (toward first column)
    double pup[3] = {0.0,0.0,0.0}; // Panel up (toward top row)
    double vup[3] = {0.0,0.0,1.0};

//    if (mConfig.ProjectionInfo.bCorrectTilt) { // check the sign and the pivot (OK pivot)
//        vup[1] = sin(-mConfig.ProjectionInfo.fTiltAngle*dPi/180);
//        vup[2] = cos(-mConfig.ProjectionInfo.fTiltAngle*dPi/180);
//    }
//    else {
//        vup[2] = 1.0; // hardcoded for ideal panel orientation  -- add tilt here
//    }

    double tgt[3] = {0.0,0.0,0.0};



//    Compute imager coordinate sys (nrm,pup,plt)
//       ---------------
//       nrm = cam - tgt
//       plt = nrm x vup
//       pup = plt x nrm
//       ---------------


    nrm[0] = cam[0]-tgt[0];
    nrm[1] = cam[1]-tgt[1];
    nrm[2] = cam[2]-tgt[2];




    double norm_nrm = sqrt(nrm[0]*nrm[0]+nrm[1]*nrm[1]+nrm[2]*nrm[2]);

    nrm[0] /= norm_nrm;
    nrm[1] /= norm_nrm;
    nrm[2] /= norm_nrm;



//    std::cout << angles << std::endl;


//            vec3_cross (plt, nrm, vup); // cross product
//            vec3_normalize1 (plt);

    plt[0] =  nrm[1]*vup[2]-nrm[2]*vup[1];
    plt[1] =  nrm[2]*vup[0]-nrm[0]*vup[2];
    plt[2] =  nrm[0]*vup[1]-nrm[1]*vup[0];

    double norm_plt = sqrt(plt[0]*plt[0]+plt[1]*plt[1]+plt[2]*plt[2]);

    plt[0] /= norm_plt;
    plt[1] /= norm_plt;
    plt[2] /= norm_plt;

//        vec3_cross (pup, plt, nrm);
//        vec3_normalize1 (pup);

    pup[0] =  plt[1]*nrm[2]-plt[2]*nrm[1];
    pup[1] =  plt[2]*nrm[0]-plt[0]*nrm[2];
    pup[2] =  plt[0]*nrm[1]-plt[1]*nrm[0];

    double norm_pup = sqrt(pup[0]*pup[0]+pup[1]*pup[1]+pup[2]*pup[2]);

    pup[0] /= norm_pup;
    pup[1] /= norm_pup;
    pup[2] /= norm_pup;


//        printf ("CAM = %g %g %g\n", cam[0], cam[1], cam[2]);
//        printf ("TGT = %g %g %g\n", tgt[0], tgt[1], tgt[2]);
//        printf ("NRM = %g %g %g\n", nrm[0], nrm[1], nrm[2]);
//        printf ("PLT = %g %g %g\n", plt[0], plt[1], plt[2]);
//        printf ("PUP = %g %g %g\n", pup[0], pup[1], pup[2]);


    // Build extrinsic matrix - rotation part


//        vec3_invert (&pmat->extrinsic[0]);
//        vec3_invert (&pmat->extrinsic[4]);
//        vec3_invert (&pmat->extrinsic[8]);

    extrinsic[0] = -1.0*plt[0];
    extrinsic[1] = -1.0*plt[1];
    extrinsic[2] = -1.0*plt[2];

    extrinsic[4] = -1.0*pup[0];
    extrinsic[5] = -1.0*pup[1];
    extrinsic[6] = -1.0*pup[2];

    extrinsic[8] = -1.0*nrm[0];
    extrinsic[9] = -1.0*nrm[1];
    extrinsic[10] = -1.0*nrm[2];

    extrinsic[15] = 1.0;



     //  ----------------- Build extrinsic matrix - translation part

//        pmat->extrinsic[3] = vec3_dot (plt, tgt);
//        pmat->extrinsic[7] = vec3_dot (pup, tgt);
//        pmat->extrinsic[11] = vec3_dot (nrm, tgt) + pmat->sad;

        extrinsic[3] = plt[0]*tgt[0]+plt[1]*tgt[1]+plt[2]*tgt[2]-offCenter;
        extrinsic[7] = pup[0]*tgt[0]+pup[1]*tgt[1]+pup[2]*tgt[2];
        extrinsic[11] = nrm[0]*tgt[0]+nrm[1]*tgt[1]+nrm[2]*tgt[2]+sad;




//        printf ("EXTRINSIC\n%g %g %g %g\n%g %g %g %g\n"
//        "%g %g %g %g\n%g %g %g %g\n",
//        extrinsic[0], extrinsic[1],
//        extrinsic[2], extrinsic[3],
//        extrinsic[4], extrinsic[5],
//        extrinsic[6], extrinsic[7],
//        extrinsic[8], extrinsic[9],
//        extrinsic[10], extrinsic[11],
//        extrinsic[12], extrinsic[13],
//        extrinsic[14], extrinsic[15]);


        // -------------- Build intrinsic matrix


//        m_idx (pmat->intrinsic,cols,0,0) = 1 / ps[0];
//        m_idx (pmat->intrinsic,cols,1,1) = 1 / ps[1];
//        m_idx (pmat->intrinsic,cols,2,2) = 1 / sid;
    intrinsic[0] = 1/mConfig.ProjectionInfo.fResolution[0];
    intrinsic[5] = 1/mConfig.ProjectionInfo.fResolution[1];
    intrinsic[10] = 1/sid;

//        printf ("INTRINSIC\n%g %g %g %g\n%g %g %g %g\n%g %g %g %g\n",
//        intrinsic[0], intrinsic[1],
//        intrinsic[2], intrinsic[3],
//        intrinsic[4], intrinsic[5],
//        intrinsic[6], intrinsic[7],
//        intrinsic[8], intrinsic[9],
//        intrinsic[10], intrinsic[11]);



    // -------  Build Projection Geometry Matrix = intrinsic * extrinsic
   int i,j,k;

   #pragma omp parallel for
   for (i = 0; i < 3; i++) {
        for (j = 0; j < 4; j++) {
            float acc = 0.0; // why not double?
            for (k = 0; k < 4; k++) {
                acc += m_idx(intrinsic,4,i,k) * m_idx(extrinsic,4,k,j);
            }
            m_idx(proj_matrix,4,i,j) = acc;
        }
   }


}


/// Main reconstruction loop - Loop over images, and backproject each image into the volume.
size_t FDKbp::reconstruct(kipl::base::TImage<float,2> &proj, float angles, size_t nProj)
{

    // missing things todo:
    // 1. use circular mask
    // 2. check parallel cycles


//        float scale;
//        double filter_time = 0.0;
//        double backproject_time = 0.0;
//        double io_time = 0.0;
        double nrm[3] = {0.0, 0.0, 0.0};
        double proj_matrix[12] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}; /* Projection matrix */


        getProjMatrix(angles, nrm, proj_matrix); // panel oriented as z, to be used with project_volume_onto_image_c

//            /* Apply ramp filter */
//            if (parms->filter == FDK_FILTER_TYPE_RAMP) {
//                timer->start ();

      //  prepareFFT(proj.Size(0),proj.Size(1)); // Prepare and cleanup should be called outside this loop why this is commented?
        ramp_filter_tuned(proj);
      //  cleanupFFT();
//                filter_time += timer->report ();
//            }



//        REFERENCE FDK IMPLEMENTATION:
//        project_volume_onto_image_reference (proj, &proj_matrix[0], &nrm[0]);

//       DEFAULT FDK BACK PROJECTOR:                
        project_volume_onto_image_c (proj, &proj_matrix[0], nProj);


//        printf ("I/O time (total) = %g\n", io_time);
//        printf ("I/O time (per image) = %g\n", io_time / num_imgs);
//        printf ("Filter time = %g\n", filter_time);
//        printf ("Filter time (per image) = %g\n", filter_time / num_imgs);
//        printf ("Backprojection time = %g\n", backproject_time);
//        printf ("Backprojection time (per image) = %g\n",
//        backproject_time / num_imgs);

//        delete timer;


    return 0L;
}


void FDKbp::project_volume_onto_image_c(kipl::base::TImage<float, 2> &cbi,
    double *proj_matrix, size_t nProj)
{
        logger(logger.LogDebug,"Started FDK back-projector");

        long int i, j, k;

        float* img = cbct_volume.GetDataPtr();
        double *xip, *yip, *zip;
        double sad_sid_2;

        float scale = mConfig.ProjectionInfo.fSOD/mConfig.ProjectionInfo.fSDD/nProj; // compensate for resolution that is already included in weights

        // spacing of the reconstructed volume. Maximum resolution for CBCT = detector pixel spacing/ magnification.
        // magnification = SDD/SOD

        float spacing[3];
        spacing[0] = mConfig.MatrixInfo.fVoxelSize[0];
        spacing[1] = mConfig.MatrixInfo.fVoxelSize[1];
        spacing[2] = mConfig.MatrixInfo.fVoxelSize[2];

        float origin[3];

        float U = static_cast<float>(mConfig.ProjectionInfo.roi[2]-mConfig.ProjectionInfo.roi[0]);
        float V = static_cast<float>(mConfig.ProjectionInfo.roi[3]-mConfig.ProjectionInfo.roi[1]);


        origin[0] = -(U-mConfig.ProjectionInfo.fCenter)*spacing[0]-spacing[0]/2; // voi[0] and voi[2] and voi[4] are 0
        origin[1] = -(U-mConfig.ProjectionInfo.fCenter)*spacing[1]-spacing[1]/2;
        origin[2] = -(V-(mConfig.ProjectionInfo.fpPoint[1]-mConfig.ProjectionInfo.roi[1]))*spacing[2]-spacing[2]/2;

//        origin[0] = -(U-mConfig.ProjectionInfo.fCenter-mConfig.MatrixInfo.voi[0])*spacing[0]-spacing[0]/2;
//        origin[1] = -(U-mConfig.ProjectionInfo.fCenter-mConfig.MatrixInfo.voi[2])*spacing[1]-spacing[1]/2;
//        origin[2] = -(V-(mConfig.ProjectionInfo.fpPoint[1]-mConfig.ProjectionInfo.roi[1])-mConfig.MatrixInfo.voi[4])*spacing[2]-spacing[2]/2;

        float radius = static_cast<float>(volume.Size(1))*mConfig.MatrixInfo.fVoxelSize[0]/2;

        size_t CBCT_roi[4];
        CBCT_roi[0] = mConfig.ProjectionInfo.roi[0];
        CBCT_roi[2] = mConfig.ProjectionInfo.roi[2];

        if (mConfig.ProjectionInfo.fpPoint[1]>=static_cast<float>(mConfig.ProjectionInfo.roi[1]) && mConfig.ProjectionInfo.fpPoint[1]>=static_cast<float>(mConfig.ProjectionInfo.roi[3])) {
            CBCT_roi[3] = static_cast<size_t>(mConfig.ProjectionInfo.fpPoint[1]-((mConfig.ProjectionInfo.fpPoint[1]-static_cast<float>(mConfig.ProjectionInfo.roi[3]))*mConfig.MatrixInfo.fVoxelSize[0]*mConfig.ProjectionInfo.fSDD/(mConfig.ProjectionInfo.fSOD+radius))/mConfig.ProjectionInfo.fResolution[0]);
            float value = mConfig.ProjectionInfo.fpPoint[1]-((mConfig.ProjectionInfo.fpPoint[1]-static_cast<float>(mConfig.ProjectionInfo.roi[1]))*mConfig.MatrixInfo.fVoxelSize[0]*mConfig.ProjectionInfo.fSDD/(mConfig.ProjectionInfo.fSOD-radius))/mConfig.ProjectionInfo.fResolution[0];
            if(value<=0)
                CBCT_roi[1] = 0;
            else
                CBCT_roi[1] = static_cast<size_t>(mConfig.ProjectionInfo.fpPoint[1]-((mConfig.ProjectionInfo.fpPoint[1]-static_cast<float>(mConfig.ProjectionInfo.roi[1]))*mConfig.MatrixInfo.fVoxelSize[0]*mConfig.ProjectionInfo.fSDD/(mConfig.ProjectionInfo.fSOD-radius))/mConfig.ProjectionInfo.fResolution[0]);
        }

        if (mConfig.ProjectionInfo.fpPoint[1]<static_cast<float>(mConfig.ProjectionInfo.roi[1]) && mConfig.ProjectionInfo.fpPoint[1]<static_cast<float>(mConfig.ProjectionInfo.roi[3]))
        {
            float value = mConfig.ProjectionInfo.fpPoint[1]+((static_cast<float>(mConfig.ProjectionInfo.roi[1])-mConfig.ProjectionInfo.fpPoint[1])*mConfig.MatrixInfo.fVoxelSize[0]*mConfig.ProjectionInfo.fSDD/(mConfig.ProjectionInfo.fSOD+radius))/mConfig.ProjectionInfo.fResolution[0];
             CBCT_roi[1] = static_cast<size_t>(value);
             float value2 = mConfig.ProjectionInfo.fpPoint[1]+((static_cast<float>(mConfig.ProjectionInfo.roi[3])-mConfig.ProjectionInfo.fpPoint[1])*mConfig.MatrixInfo.fVoxelSize[0]*mConfig.ProjectionInfo.fSDD/(mConfig.ProjectionInfo.fSOD-radius))/mConfig.ProjectionInfo.fResolution[0];
             if (value2>=mConfig.ProjectionInfo.projection_roi[3])
                 CBCT_roi[3] = mConfig.ProjectionInfo.projection_roi[3];
             else
                 CBCT_roi[3] = static_cast<float>(value2);
        }

       if (mConfig.ProjectionInfo.fpPoint[1]>=static_cast<float>(mConfig.ProjectionInfo.roi[1]) && mConfig.ProjectionInfo.fpPoint[1]<static_cast<float>(mConfig.ProjectionInfo.roi[3]))
       {
           float value = mConfig.ProjectionInfo.fpPoint[1]-((mConfig.ProjectionInfo.fpPoint[1]-static_cast<float>(mConfig.ProjectionInfo.roi[1]))*mConfig.MatrixInfo.fVoxelSize[0]*mConfig.ProjectionInfo.fSDD/(mConfig.ProjectionInfo.fSOD-radius))/mConfig.ProjectionInfo.fResolution[0];
           if(value<=0)
               CBCT_roi[1] = 0;
           else
               CBCT_roi[1] = static_cast<size_t>(mConfig.ProjectionInfo.fpPoint[1]-((mConfig.ProjectionInfo.fpPoint[1]-static_cast<float>(mConfig.ProjectionInfo.roi[1]))*mConfig.MatrixInfo.fVoxelSize[0]*mConfig.ProjectionInfo.fSDD/(mConfig.ProjectionInfo.fSOD-radius))/mConfig.ProjectionInfo.fResolution[0]);

           float value2 = mConfig.ProjectionInfo.fpPoint[1]+((static_cast<float>(mConfig.ProjectionInfo.roi[3])-mConfig.ProjectionInfo.fpPoint[1])*mConfig.MatrixInfo.fVoxelSize[0]*mConfig.ProjectionInfo.fSDD/(mConfig.ProjectionInfo.fSOD-radius))/mConfig.ProjectionInfo.fResolution[0];
           if (value2>=mConfig.ProjectionInfo.projection_roi[3])
               CBCT_roi[3] = mConfig.ProjectionInfo.projection_roi[3];
           else
               CBCT_roi[3] = static_cast<float>(value2);
       }


       if (CBCT_roi[1]-8>=0)
           CBCT_roi[1] -=8;
       if (CBCT_roi[3]+8<=mConfig.ProjectionInfo.projection_roi[3])
           CBCT_roi[3] +=8;


//        double ic[2] = {mConfig.ProjectionInfo.fpPoint[0]-mConfig.ProjectionInfo.roi[0], mConfig.ProjectionInfo.fpPoint[1]-mConfig.ProjectionInfo.roi[1]}; // piercing point
        double ic[2] = {mConfig.ProjectionInfo.fpPoint[0]-CBCT_roi[0], mConfig.ProjectionInfo.fpPoint[1]-CBCT_roi[1]};

        // Rescale image (destructive rescaling)
//        sad_sid_2 = (pmat->sad * pmat->sad) / (pmat->sid * pmat->sid);
        sad_sid_2 = (mConfig.ProjectionInfo.fSOD * mConfig.ProjectionInfo.fSOD) / (mConfig.ProjectionInfo.fSDD * mConfig.ProjectionInfo.fSDD);



        #pragma omp parallel for
        for (i=0; i<cbi.Size(0)*cbi.Size(1); ++i) {
            cbi[i] *=sad_sid_2; 	// Speedup trick re: Kachelsreiss
            cbi[i] *=scale;         // User scaling
        }



        xip = (double*) malloc (3*volume.Size(0)*sizeof(double));
        yip = (double*) malloc (3*volume.Size(1)*sizeof(double));
        zip = (double*) malloc (3*volume.Size(2)*sizeof(double));


        // Precompute partial projections here
        #pragma omp parallel for
        for (i = 0; i < volume.Size(0); i++) {
            double x = (double) (origin[0] + i * spacing[0]);
            xip[i*3+0] = x * (proj_matrix[0] + ic[0] * proj_matrix[8]);
            xip[i*3+1] = x * (proj_matrix[4] + ic[1] * proj_matrix[8]);
            xip[i*3+2] = x * proj_matrix[8];
        }

        #pragma omp parallel for
        for (j = 0; j < volume.Size(1); j++) {
            double y = (double) (origin[1] + j * spacing[1]);
            yip[j*3+0] = y * (proj_matrix[1] + ic[0] * proj_matrix[9]);
            yip[j*3+1] = y * (proj_matrix[5] + ic[1] * proj_matrix[9]);
            yip[j*3+2] = y * proj_matrix[9];
        }

        double cor_tilted;

        #pragma omp parallel for
        for (k = 0; k < volume.Size(2); k++) {
            double z = (double) (origin[2] + k * spacing[2]);

            // not so elegant solution but it seems to work
                if (mConfig.ProjectionInfo.bCorrectTilt){
                    double pos = static_cast<double> (CBCT_roi[3])-static_cast<double>(k)-static_cast<double>(mConfig.ProjectionInfo.fTiltPivotPosition);
                    cor_tilted = tan(-mConfig.ProjectionInfo.fTiltAngle*dPi/180)*pos+mConfig.ProjectionInfo.fCenter;
                    proj_matrix[3] = ((cor_tilted-(mConfig.ProjectionInfo.fpPoint[0]-mConfig.ProjectionInfo.roi[0]))*mConfig.MatrixInfo.fVoxelSize[0])/mConfig.ProjectionInfo.fResolution[0];

                }

            zip[k*3+0] = z * (proj_matrix[2] + ic[0] * proj_matrix[10])
                + ic[0] * proj_matrix[11] + proj_matrix[3];
            zip[k*3+1] = z * (proj_matrix[6] + ic[1] * proj_matrix[10])
                + ic[1] * proj_matrix[11] + proj_matrix[7];
            zip[k*3+2] = z * proj_matrix[10] + proj_matrix[11];
        }

          /* Main loop */
//        int long p;

        #pragma omp parallel for
        for (k = 0; k < volume.Size(2); k++) {
//             p = k * volume.Size(1) * volume.Size(0);
//            int long p;
            int long j;
            for (j = 0; j < volume.Size(1); j++) {
                int long i;
                double acc2[3];
    //            static inline void vec3_add3 (double* v1, const double* v2, const double* v3) {
    //                v1[0] = v2[0] + v3[0]; v1[1] = v2[1] + v3[1]; v1[2] = v2[2] + v3[2];
    //            }
    //            vec3_add3 (acc2, &zip[3*k], &yip[3*j]);
                acc2[0] = zip[3*k]+yip[3*j];
                acc2[1] = zip[3*k+1]+yip[3*j+1];
                acc2[2] = zip[3*k+2]+yip[3*j+2];
//                int long p = k * volume.Size(1) * volume.Size(0)+j *volume.Size(0);
//                for (i = 0; i < volume.Size(0); i++) {
                int long p=k * volume.Size(1) * volume.Size(0)+j *volume.Size(0);
                for (i = mask[j].first+1; i <= mask[j].second; i++) {
                        double dw;
                        double acc3[3];
                        acc3[0] = acc2[0]+xip[3*i];
                        acc3[1] = acc2[1]+xip[3*i+1];
                        acc3[2] = acc2[2]+xip[3*i+2];
                        dw = 1.0 / acc3[2];
                        acc3[0] = acc3[0] * dw;
                        acc3[1] = acc3[1] * dw;
                        img[p+i] +=
                            dw * dw * get_pixel_value_c (cbi, acc3[1], acc3[0]);
                }
            }
        }


        free (xip);
        free (yip);
        free (zip);
}



// Reference implementation is the most straightforward implementation, also it is the slowest
void FDKbp::project_volume_onto_image_reference (
    kipl::base::TImage<float, 2> &cbi,
    double *proj_matrix,
    double *nrm
)
{
    int i, j, k, p;
    double vp[4];   /* vp = voxel position */
//    float* img = (float*) vol->img; // voxel data of the reconstructed volume -> è il mio volume.
    float *img = volume.GetDataPtr(); // do i need this?



    const size_t SizeZ  = volume.Size(0)>>2;
    __m128 column[2048]; // not used for now..

//    Proj_matrix *pmat = cbi->pmat; /* projection matrix 3D -> 2D in homogenous coordinates */

    p = 0;
    vp[3] = 1.0;
    /* Loop k (slices), j (rows), i (cols) */


    double ic[2] = {mConfig.ProjectionInfo.fpPoint[0]-mConfig.ProjectionInfo.roi[0], mConfig.ProjectionInfo.fpPoint[1]-mConfig.ProjectionInfo.roi[1]}; // piercing point


    float spacing[3];
    spacing[0] = mConfig.MatrixInfo.fVoxelSize[0];
    spacing[1] = mConfig.MatrixInfo.fVoxelSize[1];
    spacing[2] = mConfig.MatrixInfo.fVoxelSize[2];

    float U = static_cast<float>(mConfig.ProjectionInfo.roi[2]-mConfig.ProjectionInfo.roi[0]);
    float V = static_cast<float>(mConfig.ProjectionInfo.roi[3]-mConfig.ProjectionInfo.roi[1]);

    float origin[3];

    origin[0] = -(U-mConfig.ProjectionInfo.fCenter)*spacing[0]-spacing[0]/2; // voi[0] and voi[2] and voi[4] are 0
    origin[1] = -(U-mConfig.ProjectionInfo.fCenter)*spacing[1]-spacing[1]/2;
    origin[2] = -(V-(mConfig.ProjectionInfo.fpPoint[1]-mConfig.ProjectionInfo.roi[1]))*spacing[2]-spacing[2]/2;


    for (k = 0; k < volume.Size(2); k++) {

//        vp[2] = (double) (vol->origin[2] + k * vol->spacing[2]);
        vp[2] = (double) (origin[2] + k * spacing[2]);

    for (j = 0; j < volume.Size(1); j++) {
        vp[1] = (double) (origin[1] + j * spacing[1]);

        for (i = mask[j].first+1; i <= mask[j].second; i++) {

            double ip[3];        // ip = image position
            double s;            // s = projection of vp onto s axis
            vp[0] = (double) (origin[0] + i * spacing[0]);
//            mat43_mult_vec3 (ip, pmat->matrix, vp);
            // ip = matrix * vp
            ip[0] = proj_matrix[0]*vp[0]+proj_matrix[1]*vp[1]+proj_matrix[2]*vp[2]+proj_matrix[3]*vp[3];
            ip[1] = proj_matrix[4]*vp[0]+proj_matrix[5]*vp[1]+proj_matrix[6]*vp[2]+proj_matrix[7]*vp[3];
            ip[2] = proj_matrix[8]*vp[0]+proj_matrix[9]*vp[1]+proj_matrix[10]*vp[2]+proj_matrix[11]*vp[3];

//            ip[0] = pmat->ic[0] + ip[0] / ip[2];
//            ip[1] = pmat->ic[1] + ip[1] / ip[2];
                    ip[0] = ic[0] + ip[0] / ip[2];
                    ip[1] = ic[1] + ip[1] / ip[2];

            // Distance on central axis from voxel center to source
            // pmat->nrm is normal of panel

//            s = vec3_dot (pmat->nrm, vp);

            s = nrm[0]*vp[0]+nrm[1]*vp[1]+nrm[2]*vp[2];

            // Conebeam weighting factor
            s = mConfig.ProjectionInfo.fSOD - s;
            s = mConfig.ProjectionInfo.fSOD * mConfig.ProjectionInfo.fSOD / (s * s);


//            volume[p++] += scale * s * get_pixel_value_b (cbi, ip[1], ip[0]);
            p = k * volume.Size(1) * volume.Size(0)+j *volume.Size(0)+i;
            img[p] += s*get_pixel_value_b (cbi, ip[1], ip[0]);
        }


    }
    }

}

// Nearest neighbor interpolation of intensity value on image
 float FDKbp::get_pixel_value_b (kipl::base::TImage<float, 2> &cbi, double r, double c)
{
    int rr, cc;

    rr = (int)(r);
//    if (rr < 0 || rr >= cbi->dim[1]) return 0.0;
    if (rr < 0 || rr >= cbi.Size(1)) return 0.0;
    cc = (int)(c);
//    if (cc < 0 || cc >= cbi->dim[0]) return 0.0;
    if (cc < 0 || cc >= cbi.Size(0)) return 0.0;
    return cbi[rr*cbi.Size(0) + cc];
}


 // get_pixel_value_c seems to be no faster than get_pixel_value_b, despite having two fewer compares.
float FDKbp::get_pixel_value_c (kipl::base::TImage<float,2> &cbi, double r, double c)
 {
     int rr, cc;

// #if defined (commentout)
//     r += 0.5;
//     if (r < 0) return 0.0;
//     rr = (int) r;
//     if (rr >= cbi->dim[1]) return 0.0;

//     c += 0.5;
//     if (c < 0) return 0.0;
//     cc = (int) c;
//     if (cc >= cbi->dim[0]) return 0.0;
// #endif

//     r += 0.5;
//     if (r < 0) return 0.0;
//     if (r >= (double) cbi->dim[1]) return 0.0;
//     rr = (int) r;

//     c += 0.5;
//     if (c < 0) return 0.0;
//     if (c >= (double) cbi->dim[0]) return 0.0;
//     cc = (int) c;

//     return cbi->img[rr*cbi->dim[0] + cc];

     r += 0.5;
     if (r < 0) return 0.0;
     if (r >= (double) cbi.Size(1)) return 0.0;
     rr = (int) r;

     c += 0.5;
     if (c < 0) return 0.0;
     if (c >= (double) cbi.Size(0)) return 0.0;
     cc = (int) c;

//     std::cout << cbi[rr*cbi.Size(0) + cc] << std::endl;

     return cbi[rr*cbi.Size(0) + cc];

 }

//! In-place ramp filter for greyscale images
//! \param data The pixel data of the image
//! \param width The width of the image
//! \param height The height of the image
//! \template_param T The type of pixel in the image
//void
//ramp_filter (
//    float *data,
//    unsigned int width,
//    unsigned int height
//)


void FDKbp::ramp_filter(kipl::base::TImage<float, 2> &img)
{
    logger(logger.LogDebug,"Started ramp_filter");
    unsigned int i, r, c;
    unsigned int N, Npad;

    unsigned int width = img.Size(0);
    unsigned int height = img.Size(1);
    unsigned int padwidth = 2*pow(2, ceil(log(width)/log(2)));
    unsigned int pad_factor = padwidth-width;
//    unsigned int padheight = 2*pow(2, ceil(log(height)/log(2)));
    unsigned int padheight = height;
//    width = pow(2, ceil(log(width)/log(2)));
//    width *=2;
//    height = pow(2, ceil(log(height)/log(2)));
//    height *=2; // much more padding


//    std::cout << "width and height: " << width << " " << height << std::endl;

    size_t pad_dims[2] = {padwidth, height};
    kipl::base::TImage< float,2 > padImg(pad_dims);
    padImg = 0.0; // fill with zeros

    // do zero padding
    for (size_t i=0; i<height; ++i){
        memcpy(padImg.GetLinePtr(i,0)+pad_factor/2, img.GetLinePtr(i,0), sizeof(float)*width);
    }

//    kipl::io::WriteTIFF32(padImg,"padImg.tif");
//    kipl::io::WriteTIFF32(img,"img.tif");


    fftw_complex *in;
    fftw_complex *fft;
    fftw_complex *ifft;
    fftw_plan fftp;
    fftw_plan ifftp;
    double *ramp;
//    ramp = (double*) malloc (width * sizeof(double));
    ramp = (double*) malloc (padwidth * sizeof(double));


    if (!ramp) {
        printf ("Error allocating memory for ramp\n"); // to substitute with an exception
    }

    N = width * height;
    Npad = padwidth*padheight;
//    in = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * N);
//    fft = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * N);
//    ifft = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * N);
    in = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * Npad);
    fft = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * Npad);
    ifft = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * Npad);


    if (!in || !fft || !ifft) {
        printf("Error allocating memory for fft\n");
    }

    float *data = img.GetDataPtr();
    float *paddata = padImg.GetDataPtr();

//    for (r = 0; r < MARGIN; ++r)
//        memcpy (data + r * width, data + MARGIN * width,
//        width * sizeof(float));

//    for (r = height - MARGIN; r < height; ++r)
//        memcpy (data + r * width, data + (height - MARGIN - 1) * width,
//        width * sizeof(float));

//    for (r = 0; r < height; ++r) {
//        for (c = 0; c < MARGIN; ++c){
//            data[r * width + c] = data[r * width + MARGIN];
//        }
//        for (c = width - MARGIN; c < width; ++c){
//            data[r * width + c] = data[r * width + width - MARGIN - 1];
//        }
//    }

    for (r = 0; r < MARGIN; ++r)
        memcpy (paddata + r * padwidth, paddata + MARGIN * padwidth,
        padwidth * sizeof(float));

    for (r = height - MARGIN; r < height; ++r)
        memcpy (paddata + r * padwidth, paddata + (padheight - MARGIN - 1) * padwidth,
        padwidth * sizeof(float));

    for (r = 0; r < padheight; ++r) {
        for (c = 0; c < MARGIN; ++c){
            paddata[r * padwidth + c] = paddata[r * padwidth + MARGIN];
        }
        for (c = padwidth - MARGIN; c < padwidth; ++c){
            paddata[r * padwidth + c] = paddata[r * padwidth + padwidth - MARGIN - 1];
        }
    }

//    kipl::io::WriteTIFF32(padImg,"AfterStuff.tif");

//    /* Fill in ramp filter in frequency space */
//    for (i = 0; i < N; ++i) {
//        in[i][0] = (double)(data[i]);
//        // comment from here, check what happens here to understand the attenuation coefficients....
////        in[i][0] /= 65535;
////        in[i][0] = (in[i][0] == 0 ? 1 : in[i][0]);
////        in[i][0] = -log (in[i][0]);
//        // to here, to erase -log operation on projections
//        in[i][1] = 0.0;
//    }

    /* Fill in ramp filter in frequency space */
    for (i = 0; i < Npad; ++i) {
        in[i][0] = (double)(paddata[i]);
        // comment from here, check what happens here to understand the attenuation coefficients....
//        in[i][0] /= 65535;
//        in[i][0] = (in[i][0] == 0 ? 1 : in[i][0]);
//        in[i][0] = -log (in[i][0]);
        // to here, to erase -log operation on projections
        in[i][1] = 0.0;
    }



//    /* Add padding */
//    for (i = 0; i < width / 2; ++i)
//        ramp[i] = i;


//    for (i = width / 2; i < (unsigned int) width; ++i)
//        ramp[i] = width - i;

//    /* Roll off ramp filter */
//    for (i = 0; i < width; ++i)
//        ramp[i] *= (cos (i * DEGTORAD * 360 / width) + 1) / 2;

//    for (r = 0; r < height; ++r)
//    {
//    fftp = fftw_plan_dft_1d (width, in + r * width, fft + r * width,
//        FFTW_FORWARD, FFTW_ESTIMATE);
//    if (!fftp) {
//        printf ("Error creating fft plan\n");
//    }

//    ifftp = fftw_plan_dft_1d (width, fft + r * width, ifft + r * width,
//        FFTW_BACKWARD, FFTW_ESTIMATE);
//    if (!ifftp) {
//        printf ("Error creating ifft plan\n");
//    }

    /* Add padding */
    for (i = 0; i < padwidth / 2; ++i)
        ramp[i] = i;


    for (i = padwidth / 2; i < (unsigned int) padwidth; ++i)
        ramp[i] = padwidth - i;

    /* Roll off ramp filter */
    for (i = 0; i < padwidth; ++i)
        ramp[i] *= (cos (i * DEGTORAD * 360 / padwidth) + 1) / 2;

    for (r = 0; r < padheight; ++r)
    {
    fftp = fftw_plan_dft_1d (padwidth, in + r * padwidth, fft + r * padwidth,
        FFTW_FORWARD, FFTW_ESTIMATE);
    if (!fftp) {
        printf ("Error creating fft plan\n");
    }

    ifftp = fftw_plan_dft_1d (padwidth, fft + r * padwidth, ifft + r * padwidth,
        FFTW_BACKWARD, FFTW_ESTIMATE);
    if (!ifftp) {
        printf ("Error creating ifft plan\n");
    }

        fftw_execute (fftp);

//        // Apply ramp
//        for (c = 0; c < width; ++c) {
//            fft[r * width + c][0] *= ramp[c];
//            fft[r * width + c][1] *= ramp[c];
//        }

        // Apply ramp
        for (c = 0; c < padwidth; ++c) {
            fft[r * padwidth + c][0] *= ramp[c];
            fft[r * padwidth + c][1] *= ramp[c];
        }

        // Add apodization

        fftw_execute (ifftp);

    fftw_destroy_plan (fftp);
    fftw_destroy_plan (ifftp);
    }

//    for (i = 0; i < N; ++i)
//        ifft[i][0] /= width;

//    for (i = 0; i < N; ++i)
//        data[i] = (float)(ifft[i][0]);

    for (i = 0; i < Npad; ++i)
        ifft[i][0] /= padwidth;

    for (i = 0; i < Npad; ++i)
        paddata[i] = (float)(ifft[i][0]);

    // fo back to original dimension

    for (i=0; i<height; ++i){
        memcpy(img.GetLinePtr(i,0), padImg.GetLinePtr(i,0)+pad_factor/2, sizeof(float)*width);
    }



    fftw_free (in);
    fftw_free (fft);
    fftw_free (ifft);
    free (ramp);
}

void FDKbp::ramp_filter_tuned(kipl::base::TImage<float, 2> &img)
{
    std::ostringstream msg;
    logger(logger.LogDebug,"Started tuned ramp_filter");
    unsigned int i, r, c;
    unsigned int N, Npad;

    unsigned int width      = img.Size(0);
    unsigned int height     = img.Size(1);
    unsigned int pad_factor = padwidth-width;
    unsigned int padheight  = height;
    // Comment AK: Using only the intended line will speed up things
    unsigned int firstLine  = 0;      //mConfig.ProjectionInfo.roi[1];
    unsigned int lastLine   = height; //mConfig.ProjectionInfo.roi[3];
//    unsigned int firstLine  = mConfig.ProjectionInfo.roi[1];
//    unsigned int lastLine   = mConfig.ProjectionInfo.roi[3];

    msg.str("");
    msg<<"Pad width="<<padwidth;
    logger(logger.LogDebug,msg.str());

    size_t pad_dims[2] = {padwidth, height};
    kipl::base::TImage< float,2 > padImg(pad_dims);
    padImg = 0.0f; // fill with zeros

    // do zero padding
    for (size_t i=firstLine; i<lastLine; ++i)
    {
        memcpy(padImg.GetLinePtr(i)+pad_factor/2, img.GetLinePtr(i), sizeof(float)*width);
    }


    double *ramp=nullptr;

    ramp = (double*) malloc (padwidth * sizeof(double));

    if (!ramp) {
        throw ReconException("Error allocating memory for ramp",__FILE__,__LINE__);
    }

    N = width * height;
    Npad = padwidth*padheight;

    if (!in || !fft || !ifft) {
        throw ReconException("Error allocating memory for fft buffers",__FILE__,__LINE__);
    }

    float *data = img.GetDataPtr();
    float *paddata = padImg.GetDataPtr();

    for (r = 0; r < MARGIN; ++r)
        memcpy (paddata + r * padwidth, paddata + MARGIN * padwidth,
        padwidth * sizeof(float));

    for (r = height - MARGIN; r < height; ++r)
        memcpy (paddata + r * padwidth, paddata + (padheight - MARGIN - 1) * padwidth,
        padwidth * sizeof(float));

    for (r = 0; r < padheight; ++r) {
        for (c = 0; c < MARGIN; ++c){
            paddata[r * padwidth + c] = paddata[r * padwidth + MARGIN];
        }
        for (c = padwidth - MARGIN; c < padwidth; ++c){
            paddata[r * padwidth + c] = paddata[r * padwidth + padwidth - MARGIN - 1];
        }
    }


    // Fill in ramp filter in frequency space
    for (i = 0; i < Npad; ++i) {
        in[i][0] = (double)(paddata[i]);
        in[i][1] = 0.0;
    }


    // Add padding
    for (i = 0; i < padwidth / 2; ++i)
        ramp[i] = i;


    for (i = padwidth / 2; i < (unsigned int) padwidth; ++i)
        ramp[i] = padwidth - i;

    // Roll off ramp filter
    for (i = 0; i < padwidth; ++i)
        ramp[i] *= (cos (i * DEGTORAD * 360 / padwidth) + 1) / 2;



    for (r = firstLine; r < lastLine; ++r)
    {
        // Copy padded projection line to 'in' buffer
        memcpy(in_buffer,in+r*padwidth,sizeof(fftw_complex)*padwidth);
        fftw_execute (fftp);

        // Apply ramp
        for (c = 0; c < padwidth; ++c) {
            fft[c][0] *= ramp[c];
            fft[c][1] *= ramp[c];
        }

        // Add apodization. Comment AK: This already done as roll-off ramp.

        fftw_execute (ifftp);
        memcpy(ifft+r*padwidth,ifft_buffer,sizeof(fftw_complex)*padwidth);
    }

    data[i] = (float)(ifft[i][0]);

    for (i = 0; i < Npad; ++i)
        ifft[i][0] /= padwidth;

    for (i = 0; i < Npad; ++i)
        paddata[i] = (float)(ifft[i][0]);

    // fo back to original dimension

    for (i=firstLine; i<lastLine; ++i){
        memcpy(img.GetLinePtr(i,0), padImg.GetLinePtr(i,0)+pad_factor/2, sizeof(float)*width);
    }

    free (ramp);
}

void FDKbp::prepareFFT(int width, int height)
{
    padwidth = 2*pow(2, ceil(log(width)/log(2)));
    int Npad=padwidth*height;

    in          = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * Npad);
    in_buffer   = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * padwidth);
    fft         = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * padwidth);
    ifft        = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * Npad);
    ifft_buffer = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * padwidth);

    fftp = fftw_plan_dft_1d (padwidth, in_buffer, fft, FFTW_FORWARD, FFTW_ESTIMATE);
    if (!fftp)
    {
        throw ReconException("Error creating fft plan",__FILE__,__LINE__);
    }

    ifftp = fftw_plan_dft_1d (padwidth, fft, ifft_buffer, FFTW_BACKWARD, FFTW_ESTIMATE);
    if (!ifftp)
    {
        throw ReconException("Error creating ifft plan",__FILE__,__LINE__);
    }
}

void FDKbp::cleanupFFT()
{
    fftw_destroy_plan (fftp);
    fftw_destroy_plan (ifftp);

    fftw_free (in);
    fftw_free (fft);
    fftw_free (ifft);
    fftw_free (in_buffer);
    fftw_free (ifft_buffer);
}
