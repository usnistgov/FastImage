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
// Created by anb22 on 2/14/18.
//

#ifndef FASTIMAGE_TESTEXCEPTIONS_H
#define FASTIMAGE_TESTEXCEPTIONS_H

#include <include/gtest/gtest.h>
#include "../../src/FastImage/api/FastImage.h"
#include "../../src/FastImage/TileLoaders/GrayscaleTiffTileLoader.h"

void testOutOfBounds() {
  auto tileLoader = new fi::GrayscaleTiffTileLoader<float>("mosaic.tif");
  auto fig = new fi::FastImage<float>(tileLoader, 0);
  fig->configureAndRun();
  fig->requestTile(2, 3, true);
  while (fig->isGraphProcessingTiles()) {
    auto pView = fig->getAvailableViewBlocking();
    if (pView != nullptr) {
      auto view = pView->get();
#ifdef DEBUG
      ASSERT_THROW(view->getPixel(-1, -1), fi::FastImageException);
#endif
      pView->releaseMemory();
    }
  }
  delete (fig);
}

void testCacheOutOfBounds() {
  fi::FigCache<int> cache(1);
  cache.initCache(3, 3, 16, 16);
#ifdef DEBUG
  ASSERT_THROW(cache.getLockedTile(3, 3), fi::FastImageException);
#endif
}

#endif //FASTIMAGE_TESTEXCEPTIONS_H
