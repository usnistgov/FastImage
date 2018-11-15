// NIST-developed software is provided by NIST as a public service. 
// You may use, copy and distribute copies of the  software in any  medium, 
// provided that you keep intact this entire notice. You may improve, 
// modify and create derivative works of the software or any portion of the 
// software, and you may copy and distribute such modifications or works. 
// Modified works should carry a notice stating that you changed the software 
// and should note the date and nature of any such change. Please explicitly 
// acknowledge the National Institute of Standards and Technology as the 
// source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY
// OF ANY KIND, EXPRESS, IMPLIED, IN FACT  OR ARISING BY OPERATION OF LAW, 
// INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST 
// NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION  OF THE SOFTWARE WILL 
// BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST 
// DOES NOT WARRANT  OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE 
// SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE 
// CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using 
// and distributing the software and you assume  all risks associated with 
// its use, including but not limited to the risks and costs of program 
// errors, compliance  with applicable laws, damage to or loss of data, 
// programs or equipment, and the unavailability or interruption of operation. 
// This software is not intended to be used in any situation where a failure 
// could cause risk of injury or damage to property. The software developed 
// by NIST employees is not subject to copyright protection within 
// the United States.

//
// Created by anb22 on 2/2/18.
//

#ifndef FASTIMAGE_TESTVIEWLOADER_H
#define FASTIMAGE_TESTVIEWLOADER_H

#include <cstdint>
#include <FastImage/data/ViewRequestData.h>
#include <FastImage/data/TileRequestData.h>
#include <htgs/api/MemoryData.hpp>
#include <htgs/api/TaskGraphConf.hpp>
#include <FastImage/tasks/ViewLoader.h>
#include <htgs/api/TaskGraphRuntime.hpp>
#include <include/gtest/gtest.h>
#include <FastImage/memory/ViewAllocator.h>

std::pair<htgs::TaskGraphRuntime *,
          htgs::TaskGraphConf<fi::ViewRequestData<int>,
                              fi::TileRequestData<int> > *>
createGraphTestViewLoader(uint32_t viewWidth, uint32_t viewHeight) {
  auto graphConf = new htgs::TaskGraphConf<fi::ViewRequestData<int>,
                                           fi::TileRequestData<int> >();
  htgs::TaskGraphRuntime *runtime = nullptr;
  std::vector<uint32_t> options = {1};
  auto viewLoader = new fi::ViewLoader<int>(options);

  graphConf->setGraphConsumerTask(viewLoader);
  graphConf->addGraphProducerTask(viewLoader);
  graphConf->addMemoryManagerEdge(
      "viewMem", viewLoader,
      new fi::ViewAllocator<int>(viewHeight, viewWidth), 1, htgs::MMType::Static
  );

  runtime = new htgs::TaskGraphRuntime(graphConf);
  runtime->executeRuntime();
  return {runtime, graphConf};
}

void testViewRequestData() {
  fi::ViewRequestData<int>
      viewRequestDataUL(0, 0, 3, 3, 2, 5, 5, 15, 15, 0);
  ASSERT_EQ(viewRequestDataUL.getIndexRowCenterTile(), 0);
  ASSERT_EQ(viewRequestDataUL.getIndexColCenterTile(), 0);
  ASSERT_EQ(viewRequestDataUL.getIndexRowMinTile(), 0);
  ASSERT_EQ(viewRequestDataUL.getIndexRowMaxTile(), 2);
  ASSERT_EQ(viewRequestDataUL.getIndexColMinTile(), 0);
  ASSERT_EQ(viewRequestDataUL.getIndexColMaxTile(), 2);
  ASSERT_EQ(viewRequestDataUL.getImageWidth(), 15);
  ASSERT_EQ(viewRequestDataUL.getImageHeight(), 15);
  ASSERT_EQ(viewRequestDataUL.getMinRowFile(), 0);
  ASSERT_EQ(viewRequestDataUL.getMinColFile(), 0);
  ASSERT_EQ(viewRequestDataUL.getMaxRowFile(), 7);
  ASSERT_EQ(viewRequestDataUL.getMaxColFile(), 7);
  ASSERT_EQ(viewRequestDataUL.getTileHeight(), 5);
  ASSERT_EQ(viewRequestDataUL.getTileWidth(), 5);
  ASSERT_EQ(viewRequestDataUL.getRadius(), 2);
  ASSERT_EQ(viewRequestDataUL.getViewHeight(), 9);
  ASSERT_EQ(viewRequestDataUL.getViewWidth(), 9);
  ASSERT_EQ(viewRequestDataUL.getTopFill(), 2);
  ASSERT_EQ(viewRequestDataUL.getLeftFill(), 2);
  ASSERT_EQ(viewRequestDataUL.getBottomFill(), 0);
  ASSERT_EQ(viewRequestDataUL.getRightFill(), 0);
  ASSERT_EQ(viewRequestDataUL.getNumberTilesToLoad(), 4);

  fi::ViewRequestData<int>
      viewRequestDataBR(2, 2, 3, 3, 2, 5, 5, 15, 13, 0);
  ASSERT_EQ(viewRequestDataBR.getIndexRowCenterTile(), 2);
  ASSERT_EQ(viewRequestDataBR.getIndexColCenterTile(), 2);
  ASSERT_EQ(viewRequestDataBR.getIndexRowMinTile(), 1);
  ASSERT_EQ(viewRequestDataBR.getIndexRowMaxTile(), 3);
  ASSERT_EQ(viewRequestDataBR.getIndexColMinTile(), 1);
  ASSERT_EQ(viewRequestDataBR.getIndexColMaxTile(), 3);
  ASSERT_EQ(viewRequestDataBR.getImageWidth(), 13);
  ASSERT_EQ(viewRequestDataBR.getImageHeight(), 15);
  ASSERT_EQ(viewRequestDataBR.getMinRowFile(), 8);
  ASSERT_EQ(viewRequestDataBR.getMinColFile(), 8);
  ASSERT_EQ(viewRequestDataBR.getMaxRowFile(), 15);
  ASSERT_EQ(viewRequestDataBR.getMaxColFile(), 13);
  ASSERT_EQ(viewRequestDataBR.getTileHeight(), 5);
  ASSERT_EQ(viewRequestDataBR.getTileWidth(), 5);
  ASSERT_EQ(viewRequestDataBR.getRadius(), 2);
  ASSERT_EQ(viewRequestDataBR.getViewHeight(), 9);
  ASSERT_EQ(viewRequestDataBR.getViewWidth(), 9);
  ASSERT_EQ(viewRequestDataBR.getTopFill(), 0);
  ASSERT_EQ(viewRequestDataBR.getLeftFill(), 0);
  ASSERT_EQ(viewRequestDataBR.getBottomFill(), 2);
  ASSERT_EQ(viewRequestDataBR.getRightFill(), 4);
  ASSERT_EQ(viewRequestDataBR.getNumberTilesToLoad(), 4);
}

void testViewLoaderTileGhostUL() {
  uint32_t
      tileWidth = 0,
      tileHeight = 0,
      radius = 0,
      viewWidth = 0,
      viewHeight = 0,
      numTileHeight = 0,
      numTileWidth = 0,
      imageHeight = 0,
      imageWidth = 0,
      tileComputed = 0,
      numTileToCompute = 0;

  fi::ViewRequestData<int> *
      viewRequestData = nullptr;

  tileWidth = 5;
  tileHeight = 5;
  radius = 2;
  viewWidth = tileWidth + 2 * radius;
  viewHeight = tileHeight + 2 * radius;
  numTileHeight = 2;
  numTileWidth = 2;
  imageHeight = 10;
  imageWidth = 8;
  auto pairRuntimeGraph = createGraphTestViewLoader(viewWidth, viewHeight);
  auto runtime = pairRuntimeGraph.first;
  auto graph = pairRuntimeGraph.second;

  viewRequestData = new fi::ViewRequestData<int>(
      0, 0,
      numTileHeight, numTileWidth,
      radius, tileHeight, tileWidth,
      imageHeight, imageWidth, 0);

  numTileToCompute =
      (viewRequestData->getIndexColMaxTile()
          - viewRequestData->getIndexColMinTile())
          * (viewRequestData->getIndexRowMaxTile()
              - viewRequestData->getIndexRowMinTile());

  graph->produceData(viewRequestData);
  while (!graph->isOutputTerminated()) {
    auto tileRequestData = graph->consumeData();
    if (tileRequestData != nullptr) {
      if (tileRequestData->getIndexRowTileAsked() == 0) {
        if (tileRequestData->getIndexColTileAsked() == 0) {
          ASSERT_EQ(tileRequestData->getIndexRowTileAsked(), 0);
          ASSERT_EQ(tileRequestData->getIndexColTileAsked(), 0);
          ASSERT_EQ(tileRequestData->getRowFrom(), 0);
          ASSERT_EQ(tileRequestData->getColFrom(), 0);
          ASSERT_EQ(tileRequestData->getRowDest(), 2);
          ASSERT_EQ(tileRequestData->getColDest(), 2);
          ASSERT_EQ(tileRequestData->getHeightToCopy(), 5);
          ASSERT_EQ(tileRequestData->getWidthToCopy(), 5);
          ASSERT_EQ(tileRequestData->getTileHeight(), 5);
          ASSERT_EQ(tileRequestData->getTileWidth(), 5);
          ASSERT_EQ(tileRequestData->getViewHeight(), 9);
          ASSERT_EQ(tileRequestData->getViewWidth(), 9);
          ASSERT_EQ(tileRequestData->getTopToFill(), 2);
          ASSERT_EQ(tileRequestData->getBottomToFill(), 0);
          ASSERT_EQ(tileRequestData->getLeftToFill(), 2);
          ASSERT_EQ(tileRequestData->getRightToFill(), 0);
        } else {
          ASSERT_EQ(tileRequestData->getIndexRowTileAsked(), 0);
          ASSERT_EQ(tileRequestData->getIndexColTileAsked(), 1);
          ASSERT_EQ(tileRequestData->getRowFrom(), 0);
          ASSERT_EQ(tileRequestData->getColFrom(), 0);
          ASSERT_EQ(tileRequestData->getRowDest(), 2);
          ASSERT_EQ(tileRequestData->getColDest(), 7);
          ASSERT_EQ(tileRequestData->getHeightToCopy(), 5);
          ASSERT_EQ(tileRequestData->getWidthToCopy(), 2);
          ASSERT_EQ(tileRequestData->getTileHeight(), 5);
          ASSERT_EQ(tileRequestData->getTileWidth(), 5);
          ASSERT_EQ(tileRequestData->getViewHeight(), 9);
          ASSERT_EQ(tileRequestData->getViewWidth(), 9);
          ASSERT_EQ(tileRequestData->getTopToFill(), 2);
          ASSERT_EQ(tileRequestData->getBottomToFill(), 0);
          ASSERT_EQ(tileRequestData->getLeftToFill(), 2);
          ASSERT_EQ(tileRequestData->getRightToFill(), 0);
        }
      } else {
        if (tileRequestData->getIndexColTileAsked() == 0) {
          ASSERT_EQ(tileRequestData->getIndexRowTileAsked(), 1);
          ASSERT_EQ(tileRequestData->getIndexColTileAsked(), 0);
          ASSERT_EQ(tileRequestData->getRowFrom(), 0);
          ASSERT_EQ(tileRequestData->getColFrom(), 0);
          ASSERT_EQ(tileRequestData->getRowDest(), 7);
          ASSERT_EQ(tileRequestData->getColDest(), 2);
          ASSERT_EQ(tileRequestData->getHeightToCopy(), 2);
          ASSERT_EQ(tileRequestData->getWidthToCopy(), 5);
          ASSERT_EQ(tileRequestData->getTileHeight(), 5);
          ASSERT_EQ(tileRequestData->getTileWidth(), 5);
          ASSERT_EQ(tileRequestData->getViewHeight(), 9);
          ASSERT_EQ(tileRequestData->getViewWidth(), 9);
          ASSERT_EQ(tileRequestData->getTopToFill(), 2);
          ASSERT_EQ(tileRequestData->getBottomToFill(), 0);
          ASSERT_EQ(tileRequestData->getLeftToFill(), 2);
          ASSERT_EQ(tileRequestData->getRightToFill(), 0);
        } else {
          ASSERT_EQ(tileRequestData->getIndexRowTileAsked(), 1);
          ASSERT_EQ(tileRequestData->getIndexColTileAsked(), 1);
          ASSERT_EQ(tileRequestData->getRowFrom(), 0);
          ASSERT_EQ(tileRequestData->getColFrom(), 0);
          ASSERT_EQ(tileRequestData->getRowDest(), 7);
          ASSERT_EQ(tileRequestData->getColDest(), 7);
          ASSERT_EQ(tileRequestData->getHeightToCopy(), 2);
          ASSERT_EQ(tileRequestData->getWidthToCopy(), 2);
          ASSERT_EQ(tileRequestData->getTileHeight(), 5);
          ASSERT_EQ(tileRequestData->getTileWidth(), 5);
          ASSERT_EQ(tileRequestData->getViewHeight(), 9);
          ASSERT_EQ(tileRequestData->getViewWidth(), 9);
          ASSERT_EQ(tileRequestData->getTopToFill(), 2);
          ASSERT_EQ(tileRequestData->getBottomToFill(), 0);
          ASSERT_EQ(tileRequestData->getLeftToFill(), 2);
          ASSERT_EQ(tileRequestData->getRightToFill(), 0);
        }
      }
      ++tileComputed;
      if (tileComputed == numTileToCompute) {
        graph->finishedProducingData();
      }
    }
  }

  runtime->waitForRuntime();
  delete (runtime);
}

void testViewLoaderTileGhostBR() {
  uint32_t
      tileWidth = 0,
      tileHeight = 0,
      radius = 0,
      viewWidth = 0,
      viewHeight = 0,
      numTileHeight = 0,
      numTileWidth = 0,
      imageHeight = 0,
      imageWidth = 0,
      tileComputed = 0,
      numTileToCompute = 0;

  fi::ViewRequestData<int> *viewRequestData = nullptr;

  tileWidth = 5;
  tileHeight = 5;
  radius = 2;
  viewWidth = tileWidth + 2 * radius;
  viewHeight = tileHeight + 2 * radius;
  numTileHeight = 2;
  numTileWidth = 2;
  imageHeight = 10;
  imageWidth = 8;
  auto pairRuntimeGraph = createGraphTestViewLoader(viewWidth, viewHeight);
  auto runtime = pairRuntimeGraph.first;
  auto graph = pairRuntimeGraph.second;

  viewRequestData = new fi::ViewRequestData<int>(
      1, 1,
      numTileHeight, numTileWidth,
      radius, tileHeight, tileWidth,
      imageHeight, imageWidth, 0);

  numTileToCompute =
      (viewRequestData->getIndexColMaxTile()
          - viewRequestData->getIndexColMinTile())
          * (viewRequestData->getIndexRowMaxTile()
              - viewRequestData->getIndexRowMinTile());

  graph->produceData(viewRequestData);
  while (!graph->isOutputTerminated()) {
    auto tileRequestData = graph->consumeData();
    if (tileRequestData != nullptr) {
      if (tileRequestData->getIndexRowTileAsked() == 0) {
        if (tileRequestData->getIndexColTileAsked() == 0) {
          ASSERT_EQ(tileRequestData->getIndexRowTileAsked(), 0);
          ASSERT_EQ(tileRequestData->getIndexColTileAsked(), 0);
          ASSERT_EQ(tileRequestData->getRowFrom(), 3);
          ASSERT_EQ(tileRequestData->getColFrom(), 3);
          ASSERT_EQ(tileRequestData->getRowDest(), 0);
          ASSERT_EQ(tileRequestData->getColDest(), 0);
          ASSERT_EQ(tileRequestData->getHeightToCopy(), 2);
          ASSERT_EQ(tileRequestData->getWidthToCopy(), 2);
          ASSERT_EQ(tileRequestData->getTileHeight(), 5);
          ASSERT_EQ(tileRequestData->getTileWidth(), 5);
          ASSERT_EQ(tileRequestData->getViewHeight(), 9);
          ASSERT_EQ(tileRequestData->getViewWidth(), 9);
          ASSERT_EQ(tileRequestData->getTopToFill(), 0);
          ASSERT_EQ(tileRequestData->getBottomToFill(), 2);
          ASSERT_EQ(tileRequestData->getLeftToFill(), 0);
          ASSERT_EQ(tileRequestData->getRightToFill(), 4);
        } else {
          ASSERT_EQ(tileRequestData->getIndexRowTileAsked(), 0);
          ASSERT_EQ(tileRequestData->getIndexColTileAsked(), 1);
          ASSERT_EQ(tileRequestData->getRowFrom(), 3);
          ASSERT_EQ(tileRequestData->getColFrom(), 0);
          ASSERT_EQ(tileRequestData->getRowDest(), 0);
          ASSERT_EQ(tileRequestData->getColDest(), 2);
          ASSERT_EQ(tileRequestData->getHeightToCopy(), 2);
          ASSERT_EQ(tileRequestData->getWidthToCopy(), 3);
          ASSERT_EQ(tileRequestData->getTileHeight(), 5);
          ASSERT_EQ(tileRequestData->getTileWidth(), 5);
          ASSERT_EQ(tileRequestData->getViewHeight(), 9);
          ASSERT_EQ(tileRequestData->getViewWidth(), 9);
          ASSERT_EQ(tileRequestData->getTopToFill(), 0);
          ASSERT_EQ(tileRequestData->getBottomToFill(), 2);
          ASSERT_EQ(tileRequestData->getLeftToFill(), 0);
          ASSERT_EQ(tileRequestData->getRightToFill(), 4);
        }
      } else {
        if (tileRequestData->getIndexColTileAsked() == 0) {
          ASSERT_EQ(tileRequestData->getIndexRowTileAsked(), 1);
          ASSERT_EQ(tileRequestData->getIndexColTileAsked(), 0);
          ASSERT_EQ(tileRequestData->getRowFrom(), 0);
          ASSERT_EQ(tileRequestData->getColFrom(), 3);
          ASSERT_EQ(tileRequestData->getRowDest(), 2);
          ASSERT_EQ(tileRequestData->getColDest(), 0);
          ASSERT_EQ(tileRequestData->getHeightToCopy(), 5);
          ASSERT_EQ(tileRequestData->getWidthToCopy(), 2);
          ASSERT_EQ(tileRequestData->getTileHeight(), 5);
          ASSERT_EQ(tileRequestData->getTileWidth(), 5);
          ASSERT_EQ(tileRequestData->getViewHeight(), 9);
          ASSERT_EQ(tileRequestData->getViewWidth(), 9);
          ASSERT_EQ(tileRequestData->getTopToFill(), 0);
          ASSERT_EQ(tileRequestData->getBottomToFill(), 2);
          ASSERT_EQ(tileRequestData->getLeftToFill(), 0);
          ASSERT_EQ(tileRequestData->getRightToFill(), 4);
        } else {
          ASSERT_EQ(tileRequestData->getIndexRowTileAsked(), 1);
          ASSERT_EQ(tileRequestData->getIndexColTileAsked(), 1);
          ASSERT_EQ(tileRequestData->getRowFrom(), 0);
          ASSERT_EQ(tileRequestData->getColFrom(), 0);
          ASSERT_EQ(tileRequestData->getRowDest(), 2);
          ASSERT_EQ(tileRequestData->getColDest(), 2);
          ASSERT_EQ(tileRequestData->getHeightToCopy(), 5);
          ASSERT_EQ(tileRequestData->getWidthToCopy(), 3);
          ASSERT_EQ(tileRequestData->getTileHeight(), 5);
          ASSERT_EQ(tileRequestData->getTileWidth(), 5);
          ASSERT_EQ(tileRequestData->getViewHeight(), 9);
          ASSERT_EQ(tileRequestData->getViewWidth(), 9);
          ASSERT_EQ(tileRequestData->getTopToFill(), 0);
          ASSERT_EQ(tileRequestData->getBottomToFill(), 2);
          ASSERT_EQ(tileRequestData->getLeftToFill(), 0);
          ASSERT_EQ(tileRequestData->getRightToFill(), 4);
        }
      }
      ++tileComputed;
      if (tileComputed == numTileToCompute) {
        graph->finishedProducingData();
      }
    }
  }

  runtime->waitForRuntime();
  delete (runtime);
}

#endif //FASTIMAGE_TESTVIEWLOADER_H
