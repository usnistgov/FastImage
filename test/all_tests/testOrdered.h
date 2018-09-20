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
// Created by anb22 on 7/26/18.
//

#ifndef FASTIMAGE_TESTORDERED_H
#define FASTIMAGE_TESTORDERED_H

#include <iostream>
#include "FastImage/api/FastImage.h"
#include "FastImage/TileLoadersExamples/TiffTileLoader.h"
bool testOrdered() {
  uint32_t
      prevRow = 0,
      prevCol = 0,
      row = 0,
      col = 0;

  auto
      *orderedFi = new fi::FastImage<uint32_t>(new fi::TiffTileLoader<uint32_t>(
      "mosaic.tif",
      50), 0);

  bool pass = true;

  orderedFi->getFastImageOptions()->setPreserveOrder(true);
  orderedFi->getFastImageOptions()->setTraversalType(
      fi::TraversalType::DIAGONAL);
  orderedFi->getFastImageOptions()->setNumberOfViewParallel(50);
  orderedFi->configureAndRun();
  orderedFi->requestAllTiles(false);
  orderedFi->requestAllTiles(false);
  orderedFi->requestAllTiles(true);

  while (orderedFi->isGraphProcessingTiles()) {
    auto pView = orderedFi->getAvailableViewBlocking();
    if (pView != nullptr) {
      auto view = pView->get();
      row = view->getRow();
      col = view->getCol();

      if (row > prevRow || row == 0) {
        prevCol = 0;
      }
      if (col < prevCol) {
        pass = false;
      }

      prevRow = row;
      prevCol = col;
      pView->releaseMemory();
    }
  }
  orderedFi->waitForGraphComplete();
  delete (orderedFi);
  return pass;
}

#endif //FASTIMAGE_TESTORDERED_H
