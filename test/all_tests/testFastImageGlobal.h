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

#ifndef FASTIMAGE_TESTFASTIMAGEGLOBAL_H
#define FASTIMAGE_TESTFASTIMAGEGLOBAL_H

#include <cstdint>
#include <FastImage/api/FastImage.h>
#include <FastImage/FeatureCollection/FeatureCollection.h>
#include <include/gtest/gtest.h>
#include "Statistics.h"

void testWholeImage() {

  auto tileLoader = new fi::TiffTileLoader<int>("mosaic.tif");
  auto fi = new fi::FastImage<int>(tileLoader, 0);

  int
      pixValue = 0;

  long long
      sum = 0,
      numberPixels = 0,
      sumSquare = 0;

  long double
      mean = 0,
      stdv = 0;

  Statistics results;

  fi->configureAndRun();
  fi->requestAllTiles(true);
  while (fi->isGraphProcessingTiles()) {
    auto pView = fi->getAvailableViewBlocking();
    if (pView != nullptr) {
      auto view = pView->get();
      sum = 0;
      sumSquare = 0;

      for (int32_t r = 0; r < view->getTileHeight(); ++r) {
        for (int32_t c = 0; c < view->getTileWidth(); ++c) {
          pixValue = view->getPixel(r, c);
          sum += pixValue;
          sumSquare += pixValue * pixValue;
        }

      }

      numberPixels = view->getTileHeight() * view->getTileWidth();
      mean = (long double) sum / numberPixels;
      stdv = std::sqrt(
          ((long double) sumSquare - (long double) sum * (long double) sum
              / (long double) numberPixels) /
              ((long double) numberPixels - 1)
      );
      results.addStatistic(mean, stdv, numberPixels);
      pView->releaseMemory();
    }
  }
  fi->waitForGraphComplete();

  ASSERT_NEAR(results.getGlobalMean(), 115.6, 0.1);
  ASSERT_NEAR(results.getGlobalStdv(), 126.9, 0.1);

  delete fi;

}

void testPartImage() {
  fc::FeatureCollection featureCollection;
  auto tileLoader = new fi::TiffTileLoader<float>("mosaic.tif");
  auto fi = new fi::FastImage<float>(tileLoader, 0);

  uint32_t
      minRow = 0,
      maxRow = 0,
      minCol = 0,
      maxCol = 0;

  long long
      count = 0;

  long double
      mean = 0,
      stdv = 0,
      sum = 0,
      sumSquare = 0,
      pixValue = 0;

  Statistics results;

  fi->configureAndRun();

  featureCollection.deserialize("fc_mosaic.serial");

  for (const auto &feature : featureCollection) {
    sum = 0;
    sumSquare = 0;
    count = 0;

    fi->requestFeature(feature);
    while (!fi->isFeatureDone()) {

      auto view = fi->getAvailableViewBlocking();
      if (view != nullptr) {
        minRow = std::max(view->get()->getGlobalYOffset(),
                          feature.getBoundingBox().getUpperLeftRow());
        maxRow = std::min(
            view->get()->getGlobalYOffset() + view->get()->getTileHeight(),
            feature.getBoundingBox().getBottomRightRow());
        minCol = std::max(view->get()->getGlobalXOffset(),
                          feature.getBoundingBox().getUpperLeftCol());
        maxCol = std::min(
            view->get()->getGlobalXOffset() + view->get()->getTileWidth(),
            feature.getBoundingBox().getBottomRightCol());

        for (auto row = minRow; row < maxRow; ++row) {
          for (auto col = minCol; col < maxCol; ++col) {
            if (feature.isInBitMask(row, col)) {
              pixValue = view->get()->getPixel(
                  row - view->get()->getGlobalYOffset(),
                  col - view->get()->getGlobalXOffset());
              sum += pixValue;
              sumSquare += pixValue * pixValue;
              count++;
            }
          }
        }
        view->releaseMemory();
      }
    }
    if (count > 0) {
      mean = sum / count;
      stdv = std::sqrt((sumSquare / count) - (mean * mean));
      results.addStatistic(mean, stdv, count);
    }

  }
  fi->finishedRequestingTiles();

  ASSERT_NEAR(results.getGlobalMean(), 115.6, 0.1);
  ASSERT_NEAR(results.getGlobalStdv(), 126.9, 0.1);
  fi->waitForGraphComplete();
  delete fi;
}

void testSingleTile() {
  fc::FeatureCollection featureCollection;
  auto tileLoader = new fi::TiffTileLoader<uint8_t>("mosaic.tif");
  auto fi = new fi::FastImage<uint8_t>(tileLoader, 0);
  uint8_t
      pixelValue;
  uint32_t
      sum = 0,
      sumSquare = 0,
      numberPixels = 0;
  double
      mean = 0,
      stdv = 0;
  fi->configureAndRun();
  fi->requestTile(0, 0, false, 0);
  fi->requestTile(0, 1, true, 0);

  auto pView = fi->getAvailableViewBlocking();
  auto view = pView->get();
  for (uint32_t row = 0; row < view->getTileHeight(); ++row) {
    for (uint32_t col = 0; col < view->getTileWidth(); ++col) {
      pixelValue = view->getPixel(row, col);
      sum += pixelValue;
      sumSquare += pixelValue * pixelValue;
    }
  }

  numberPixels =
      (uint32_t) view->getTileWidth() * (uint32_t) view->getTileHeight();
  mean = (double) sum / (double) numberPixels;
  stdv = std::sqrt((sumSquare / numberPixels) - (mean * mean));
  pView->releaseMemory();
  ASSERT_NEAR(mean, 0, 0.1);
  ASSERT_NEAR(stdv, 0, 0.1);

  sum = 0;
  sumSquare = 0;
  pView = fi->getAvailableViewBlocking();
  view = pView->get();
  for (uint32_t row = 0; row < view->getTileHeight(); ++row) {
    for (uint32_t col = 0; col < view->getTileWidth(); ++col) {
      pixelValue = view->getPixel(row, col);
      sum += pixelValue;
      sumSquare += pixelValue * pixelValue;
    }
  }

  numberPixels =
      (uint32_t) view->getTileWidth() * (uint32_t) view->getTileHeight();
  mean = (double) sum / (double) numberPixels;
  stdv = std::sqrt((sumSquare / numberPixels) - (mean * mean));
  pView->releaseMemory();
  ASSERT_NEAR(mean, 255, 0.1);
  ASSERT_NEAR(stdv, 0, 0.1);

  fi->waitForGraphComplete();
  delete fi;

}

#endif //FASTIMAGE_TESTFASTIMAGEGLOBAL_H
