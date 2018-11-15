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
// Created by anb22 on 2/13/18.
//

#ifndef FASTIMAGE_TESTTILELOADER_H
#define FASTIMAGE_TESTTILELOADER_H

#include <FastImage/tasks/ViewLoader.h>
#include <htgs/api/TaskGraphConf.hpp>
#include <htgs/api/TaskGraphRuntime.hpp>
#include <FastImage/memory/ViewAllocator.h>
#include <FastImage/TileLoaders/GrayscaleTiffTileLoader.h>
#include <include/gtest/gtest.h>

std::pair<htgs::TaskGraphRuntime *,
          htgs::TaskGraphConf<fi::ViewRequestData<int>,
                              fi::TileRequestData<int> > *>
createGraphTestTileLoader(fi::ATileLoader<int> *tileLoader) {
  auto graphConf = new htgs::TaskGraphConf<fi::ViewRequestData<int>,
                                           fi::TileRequestData<int> >();
  htgs::TaskGraphRuntime *runtime = nullptr;
  std::vector<uint32_t> options = {1};
  auto viewLoader = new fi::ViewLoader<int>(options);

  graphConf->setGraphConsumerTask(viewLoader);
  graphConf->addEdge(viewLoader, tileLoader);
  graphConf->addGraphProducerTask(tileLoader);
  graphConf->addMemoryManagerEdge(
      "viewMem",
      viewLoader,
      new fi::ViewAllocator<int>(tileLoader->getTileHeight(),
                                 tileLoader->getTileWidth()),
      1,
      htgs::MMType::Static
  );

  runtime = new htgs::TaskGraphRuntime(graphConf);
  runtime->executeRuntime();
  return {runtime, graphConf};
}

void testTileLoading() {
  std::vector<fi::FigCache<int> *> vCaches;
  auto cache = new fi::FigCache<int>(1);
  auto tileLoader = new fi::GrayscaleTiffTileLoader<int>("mosaic.tif");
  vCaches.push_back(cache);
  tileLoader->setCache(vCaches);
  auto pairRuntimeGraph = createGraphTestTileLoader(tileLoader);
  auto runtime = pairRuntimeGraph.first;
  auto graph = pairRuntimeGraph.second;
  auto numberTilesHeight =
      (uint32_t) ceil((double) tileLoader->getImageHeight()
                          / (double) tileLoader->getTileHeight()),
      numberTilesWidth =
      (uint32_t) ceil((double) tileLoader->getImageWidth()
                          / (double) tileLoader->getTileWidth());

  auto viewRequestData0 = new fi::ViewRequestData<int>(
      0, 0,
      numberTilesHeight, numberTilesWidth,
      0, tileLoader->getTileHeight(), tileLoader->getTileWidth(),
      tileLoader->getImageHeight(), tileLoader->getImageWidth(), 0),
      viewRequestData1 = new fi::ViewRequestData<int>(
      0, 1,
      numberTilesHeight, numberTilesWidth,
      0, tileLoader->getTileHeight(), tileLoader->getTileWidth(),
      tileLoader->getImageHeight(), tileLoader->getImageWidth(), 0),
      viewRequestData2 = new fi::ViewRequestData<int>(
      2, 3,
      numberTilesHeight, numberTilesWidth,
      0, tileLoader->getTileHeight(), tileLoader->getTileWidth(),
      tileLoader->getImageHeight(), tileLoader->getImageWidth(), 0);

  int turn = 0;
  cache->initCache(numberTilesHeight,
                   numberTilesWidth,
                   tileLoader->getTileHeight(),
                   tileLoader->getTileWidth());
  graph->produceData(viewRequestData0);
  graph->produceData(viewRequestData1);
  graph->produceData(viewRequestData2);
  while (!graph->isOutputTerminated()) {
    auto tileRequestData = graph->consumeData();
    if (tileRequestData != nullptr) {
      auto view = tileRequestData->getViewData()->get();
      if (tileRequestData->getIndexColTileAsked() == 0) {
        for (int i = 0; i < 16 * 16; ++i) {
          ASSERT_EQ(view->getData()[i], 0);
        }
      } else {
        for (int i = 0; i < 16 * 16; ++i) {
          ASSERT_EQ(view->getData()[i], 255);
        }
      }
      tileRequestData->getViewData()->releaseMemory();
      ++turn;
      if (turn == 3) {
        graph->finishedProducingData();
      }
    }
  }

  runtime->waitForRuntime();
  delete (cache);
  delete (runtime);
}

#endif //FASTIMAGE_TESTTILELOADER_H
