//##########################################################################
//#                                                                        #
//#                     AICEDRONE PLUGIN: qAicedrone                       #
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

#ifndef QAICEDRONETOOLSDEFINITIONS_H
#define QAICEDRONETOOLSDEFINITIONS_H


#define CC_CLASSIFICATION_MODEL_DEFAULT_CLASSIFICATION_FIELD_NAME         "Classification"
#define CC_CLASSIFICATION_MODEL_ASPRS_NAME                                "ASPRS"
#define CC_CLASSIFICATION_MODEL_PERSISTENCE_TAG_ASPRS                     "qAicedroneTools/ClassificationModel_ASPRS"
#define CC_CLASSIFICATION_MODEL_BREAKWATER_CUBES_NAME                     "BreakwaterCubes"
#define CC_CLASSIFICATION_MODEL_PERSISTENCE_TAG_BREAKWATER_CUBES          "qAicedroneTools/ClassificationModel_BreakwaterCubes"
//#define CC_CLASSIFICATION_MODEL_ARCHDATASET_NAME                          "ArchDataset"
//#define CC_CLASSIFICATION_MODEL_PERSISTENCE_TAG_ARCHDATASET               "qTidopTools/ClassificationModel_ArchDataset"
#define CC_CLASSIFICATION_MODEL_RAILWAY_NAME                              "Railway"
#define CC_CLASSIFICATION_MODEL_PERSISTENCE_TAG_RAILWAY                   "qAicedroneTools/ClassificationModel_Railway"
#define CC_CLASSIFICATION_MODEL_REMOVED_NAME                              "Removed"
#define CC_CLASSIFICATION_MODEL_REMOVED_CODE                              -999
#define CC_CLASSIFICATION_MODEL_SELECTED_NAME                             "Selected"
#define CC_CLASSIFICATION_MODEL_SELECTED_CODE                             -99
#define CC_CLASSIFICATION_MODEL_TAG_STRING_SEPARATOR                      " - "
#define CC_CLASSIFICATION_MODEL_ASPRS_NOISE_CODE                          7
#define CC_CLASSIFICATION_MODEL_NOT_CLASSIFIED_NAME                             "Not classified"
#define CC_CLASSIFICATION_MODEL_ASPRS_NOT_CLASSIFIED_CODE               0
#define CC_CLASSIFICATION_MODEL_NO_ASPRS_NOT_CLASSIFIED_CODE           -1
#define CC_CLASSIFICATION_MODEL_NO_ASPRS_UNCLASSIFIED_CODE             -2
#define CC_CLASSIFICATION_MODEL_NO_ASPRS_NOISE_CODE                    10

#define QTOOLS_AUTHOR_MAIL               "david.hernandez@uclm.es"
#define QTOOLS_DATE_FORMAT               "yyyy:MM:dd"
#define QTOOLS_LICENSE_INITIAL_DATE      "2019:04:20"
#define QTOOLS_LICENSE_FINAL_DATE        "2030:02:01"
#define QTOOLS_LICENSE_DATE_NUMBER_OF_ITERATIONS       3

#endif // QAICEDRONETOOLSDEFINITIONS_H
