//##########################################################################
//#                                                                        #
//#                     TIDOPTOOLS PLUGIN: qTidopTools                     #
//#                                                                        #
//#  This program is free software; you can redistribute it and/or modify  #
//#  it under the terms of the GNU General Public License as published by  #
//#  the Free Software Foundation; version 2 or later of the License.      #
//#                                                                        #
//#  This program is distributed in the hope that it will be useful,       #
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          #
//#  GNU General Public License for more details.                          #
//#                                                                        #
//#      COPYRIGHT: TIDOP-USAL / PAFYC-UCLM                                #
//#                                                                        #
//##########################################################################

#ifndef CC_TOOLS_RANDOM_FOREST_CLASSIFICATION_DEFINITIONS_H
#define CC_TOOLS_RANDOM_FOREST_CLASSIFICATION_DEFINITIONS_H

#define RFC_PARAMETERS_FILE_PATH            "/plugins/qTidopToolsParametersRFC.xml"

#define RFC_PROCESS_COMPUTE_FEATURES                            "1. Compute features"
#define RFC_PROCESS_COMPUTE_FEATURES_NUMBER_OF_SCALES           "CF_NumberOfScales"
#define RFC_PROCESS_COMPUTE_FEATURES_VOXEL_SIZE                 "CF_VoxelSize"
#define RFC_PROCESS_COMPUTE_FEATURES_COLOR                      "CF_Color"
#define RFC_PROCESS_COMPUTE_FEATURES_OUTPUT_FILE                "CF_OutputFile"
#define RFC_PROCESS_COMPUTE_FEATURES_MINIMUM_NUMBER_OF_FEATURES 5

#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL                                    "1. Compute features (no CGAL)"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_LOCAL_NEIGHBORHOOD_RADIUS          "CFNCGAL_LocalNeighborhoodRadius"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_OUTPUT_FILE                        "CFNCGAL_OutputFile"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_COLOR                              "CFNCGAL_Color"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_ROUGHNESS_DIRECTION                "CFNCGAL_RoughnessDirection"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_ROUGHNESS                          "CFNCGAL_Roughness"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_MOMENT_ORDER_1                     "CFNCGAL_MomentOrder1"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_MEAN_CURVATURE                     "CFNCGAL_MeanCurvature"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_GAUSSIAN_CURVATURE                 "CFNCGAL_GaussianCurvature"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_NORMAL_CHANGE_RATE                 "CFNCGAL_NormalChangeRate"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_DENSITY_KNN                        "CFNCGAL_DensityKnn"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_DENSITY_2D                         "CFNCGAL_Density2d"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_DENSITY_3D                         "CFNCGAL_Density3d"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_EIGEN_VALUES_SUM                   "CFNCGAL_EigenValuesSum"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_OMNIVARIANCE                       "CFNCGAL_Omnivariance"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_EIGEN_ENTROPY                      "CFNCGAL_EigenEntropy"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_ANISOTROPY                         "CFNCGAL_Anisotropy"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_PLANARITY                          "CFNCGAL_Planarity"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_LINEARITY                          "CFNCGAL_Linearity"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_PCA1                               "CFNCGAL_PCA1"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_PCA2                               "CFNCGAL_PCA2"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_SURFACE_VARIATION                  "CFNCGAL_SurfaceVariation"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_SPHERICITY                         "CFNCGAL_Sphericity"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_VERTICALITY                        "CFNCGAL_Verticality"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_EIGEN_VALUE_1                      "CFNCGAL_EigenValue1"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_EIGEN_VALUE_2                      "CFNCGAL_EigenValue2"
#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_EIGEN_VALUE_3                      "CFNCGAL_EigenValue3"
//#define RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_                "CFNCGAL_"


#define RFC_PROCESS_TRAINING                                    "2. Training"
#define RFC_PROCESS_TRAINING_PROCESS_ONLY_UNLOCKED_CLASSES      "TR_ProcessOnlyUnlockedClasses"
#define RFC_PROCESS_TRAINING_PROCESS_OUTPUT_TRAINING_FILE       "TR_OutputTrainingFile"
#define RFC_PROCESS_TRAINING_PROCESS_NUMBER_OF_TREES            "TR_NumberOfTrees"
#define RFC_PROCESS_TRAINING_PROCESS_MAXIMUM_DEEP               "TR_MaximumDeep"
#define RFC_PROCESS_TRAINING_AUXLIAR_SAVE_FILE_SUFFIX           ".aux"
#define RFC_PROCESS_TRAINING_PROCESS_IGNORE_INVALID_POINTS_AND_CLASSIFY_AS_NOISE               "TR_IgnorePointsWithInvalidFeaturesAndClassifyAsNoisy"

#define RFC_PROCESS_CLASSIFICATION                              "3. Classification"
#define RFC_PROCESS_CLASSIFICATION_ONLY_UNLOCKED_CLASSES        "CS_ProcessOnlyUnlockedClasses"
#define RFC_PROCESS_CLASSIFICATION_USE_TRAINING_FILE      "CS_UseTrainingFile"
#define RFC_PROCESS_CLASSIFICATION_TRAINING_FILE      "CS_TrainingFileToUse"
#define RFC_PROCESS_CLASSIFICATION_IGNORE_INVALID_POINTS_AND_CLASSIFY_AS_NOISE               "CS_IgnorePointsWithInvalidFeaturesAndClassifyAsNoisy"

#define RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING         "3. Classification with local smoothing"
#define RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING_ONLY_UNLOCKED_CLASSES        "CSWLS_ProcessOnlyUnlockedClasses"
#define RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING_USE_TRAINING_FILE      "CSWLS_UseTrainingFile"
#define RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING_TRAINING_FILE      "CSWLS_TrainingFileToUse"
#define RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING_IGNORE_INVALID_POINTS_AND_CLASSIFY_AS_NOISE               "CSWLS_IgnorePointsWithInvalidFeaturesAndClassifyAsNoisy"

#define RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT               "3. Classification with Graph Cut"
#define RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT_ONLY_UNLOCKED_CLASSES         "CSWGC_ProcessOnlyUnlockedClasses"
#define RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT_USE_TRAINING_FILE      "CSWGC_UseTrainingFile"
#define RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT_TRAINING_FILE      "CSWGC_TrainingFileToUse"
#define RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT_NUMBER_OF_SUBDIVISIONS        "CSWGC_NumberOfSubdivisions"
#define RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT_REGULARIZATION_WEIGHT         "CSWGC_RegularizationWeight"
#define RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT_IGNORE_INVALID_POINTS_AND_CLASSIFY_AS_NOISE               "CSWGC_IgnorePointsWithInvalidFeaturesAndClassifyAsNoisy"

#define RFC_PROCESS_TRAINING_CLASS_SEPARABILITY_ANALYSIS               "Training classes separability analysis"
#define RFC_PROCESS_TRAINING_CLASS_SEPARABILITY_ANALYSIS_ONLY_UNLOCKED_CLASSES         "TCSA_ProcessOnlyUnlockedClasses"
#define RFC_PROCESS_TRAINING_CLASS_SEPARABILITY_ANALYSIS_STANDARIZE_PARAMETERS         "TCSA_StandarizeParameters"
//#define RFC_PROCESS_TRAINING_CLASS_SEPARABILITY_ANALYSIS_PARAMETERS_OUTPUT_FILE         "TCSA_ParametersOutputFile"
#define RFC_PROCESS_TRAINING_CLASS_SEPARABILITY_ANALYSIS_OUTPUT_FILE         "TCSA_OutputFile"

#define RFC_PROCESS_CLASSIFICATION_NO_VALUE                     -1
#define RFC_PROCESS_CLASSIFICATION_FIELD_NAME                   "class" // para usar en algoritmo

#define TOOLS_CC_LOCAL_KNN_DENSITY_FIELD_NAME "Number of neighbors"
#define TOOLS_CC_LOCAL_SURF_DENSITY_FIELD_NAME "Surface density"
#define TOOLS_CC_LOCAL_VOL_DENSITY_FIELD_NAME "Volume density"
#define TOOLS_CC_ROUGHNESS_FIELD_NAME "Roughness"
#define TOOLS_CC_MOMENT_ORDER1_FIELD_NAME "1st order moment"
#define TOOLS_CC_CURVATURE_GAUSSIAN_FIELD_NAME "Gaussian curvature"
#define TOOLS_CC_CURVATURE_MEAN_FIELD_NAME "Mean curvature"
#define TOOLS_CC_CURVATURE_NORM_CHANGE_RATE_FIELD_NAME "Normal change rate"
#define TOOLS_CC_COLOR_RED_FIELD_NAME "color red"
#define TOOLS_CC_COLOR_GREEN_FIELD_NAME "color green"
#define TOOLS_CC_COLOR_BLUE_FIELD_NAME "color blue"


#endif
