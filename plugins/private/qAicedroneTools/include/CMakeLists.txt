
target_sources( ${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_LIST_DIR}/qToolsDefinitions.h
		${CMAKE_CURRENT_LIST_DIR}/qAicedroneTools.h
		${CMAKE_CURRENT_LIST_DIR}/qToolsAboutDlg.h
		${CMAKE_CURRENT_LIST_DIR}/ccMouseCircle.h
		${CMAKE_CURRENT_LIST_DIR}/ccToolsManualClassificationDlg.h
		${CMAKE_CURRENT_LIST_DIR}/ccClassificationModel.h
		${CMAKE_CURRENT_LIST_DIR}/ccColorStyledDelegate.h
		${CMAKE_CURRENT_LIST_DIR}/ccToolsClassificationHelper.h
		${CMAKE_CURRENT_LIST_DIR}/ccToolsRandomForestClassificationDlg.h
		${CMAKE_CURRENT_LIST_DIR}/ccToolsRandomForestClassificationDefinitions.h
		${CMAKE_CURRENT_LIST_DIR}/ccClassificationToolCGAL.h
		${CMAKE_CURRENT_LIST_DIR}/ethz_rforest/dataview.h
		${CMAKE_CURRENT_LIST_DIR}/ethz_rforest/random-forest/IO_Binary.hpp
		${CMAKE_CURRENT_LIST_DIR}/ethz_rforest/random-forest/common-libraries.hpp
		${CMAKE_CURRENT_LIST_DIR}/ethz_rforest/random-forest/forest.hpp
		${CMAKE_CURRENT_LIST_DIR}/ethz_rforest/random-forest/node.hpp
		${CMAKE_CURRENT_LIST_DIR}/ethz_rforest/random-forest/node-gini.hpp
		${CMAKE_CURRENT_LIST_DIR}/ethz_rforest/random-forest/tree.hpp
		${CMAKE_CURRENT_LIST_DIR}/ccToolsETHZRandomForest.h
		${CMAKE_CURRENT_LIST_DIR}/ccPointCloudTilesFileDlg.h
)

target_include_directories( ${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)
