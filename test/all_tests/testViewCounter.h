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

#ifndef FASTIMAGE_TESTVIEWCOUNTER_H
#define FASTIMAGE_TESTVIEWCOUNTER_H

#include <cstdint>
#include "FastImage/api/FastImage.h"
#include "FastImage/TileLoaders/GrayscaleTiffTileLoader.h"
#include <include/gtest/gtest.h>

void testViewCounterNoRadius() {
  auto tileLoader = new fi::GrayscaleTiffTileLoader <uint8_t>("mosaic.tif");
  auto fi = new fi::FastImage<uint8_t>(tileLoader, 0);
  fi->configureAndRun();
  fi->requestTile(0, 0, true);
  while (fi->isGraphProcessingTiles()) {
    auto pView = fi->getAvailableViewBlocking();
    if (pView != nullptr) {
      auto view = pView->get();
      ASSERT_EQ(view->getViewHeight(), 16);
      ASSERT_EQ(view->getViewWidth(), 16);
      ASSERT_EQ(view->getPointerTile(), view->getData());

      pView->releaseMemory();
    }
  }
  fi->waitForGraphComplete();
  delete (fi);
}

void testViewCounterRadiusUL() {
  auto tileLoader = new fi::GrayscaleTiffTileLoader<uint8_t>("mosaic.tif");
  auto fi = new fi::FastImage<uint8_t>(tileLoader, 18);
  fi->configureAndRun();
  fi->requestTile(0, 0, true);
  while (fi->isGraphProcessingTiles()) {
    auto pView = fi->getAvailableViewBlocking();
    if (pView != nullptr) {
      auto view = pView->get();
      ASSERT_EQ(view->getViewHeight(), 16 + 18 * 2);
      ASSERT_EQ(view->getViewWidth(), 16 + 18 * 2);
      ASSERT_EQ(0, view->getData()[0]);
      ASSERT_EQ(0, view->getData()[18]);
      ASSERT_EQ(255, view->getData()[34]);
      ASSERT_EQ(255, view->getData()[(18 + 16) * (16 + 18 * 2)]);
      pView->releaseMemory();
    }
  }
  delete (fi);
}

void testViewCounterRadiusBR() {
  auto fi =
      new fi::FastImage<uint8_t>(new fi::GrayscaleTiffTileLoader<uint8_t>("mosaic.tif"),
                                 14);
  fi->configureAndRun();
  fi->requestTile(2, 3, true);
  while (fi->isGraphProcessingTiles()) {
    auto pView = fi->getAvailableViewBlocking();
    if (pView != nullptr) {
      auto view = pView->get();
      ASSERT_EQ(view->getPixel(-14, -14), 255);
      ASSERT_EQ(view->getPixel(-14, 0), 0);
      ASSERT_EQ(view->getPixel(-14, 16), 0);
      ASSERT_EQ(view->getPixel(0, -14), 0);
      ASSERT_EQ(view->getPixel(0, 0), 255);
      ASSERT_EQ(view->getPixel(0, 16), 255);
      ASSERT_EQ(view->getPixel(16, -14), 0);
      ASSERT_EQ(view->getPixel(16, 0), 255);
      ASSERT_EQ(view->getPixel(16, 16), 255);
      pView->releaseMemory();
    }
  }
  delete (fi);
}

#endif //FASTIMAGE_TESTVIEWCOUNTER_H
