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
// Created by anb22 on 2/8/18.
//

#ifndef FASTIMAGE_TESTFEATURECOLLECTION_H
#define FASTIMAGE_TESTFEATURECOLLECTION_H

#include <gtest/gtest.h>
#include "FastImage/api/FastImage.h"
#include "FastImage/TileLoaders/GrayscaleTiffTileLoader.h"
#include "FastImage/FeatureCollection/FeatureCollection.h"
#include "MaskToFeatures/FloodStrategy.h"
#include "MaskToFeatures/MaskAnalyser.h"

struct Region {
  uint32_t *
      _region;

  uint32_t
      _id = 0,
      _ULR = 0,
      _ULC = 0,
      _BRR = 0,
      _BRC = 0;

  Region(uint32_t id,
         uint32_t ULR,
         uint32_t ULC,
         uint32_t BRR,
         uint32_t BRC,
         uint32_t valueFill)
      : _id(id), _ULR(ULR), _ULC(ULC), _BRR(BRR), _BRC(BRC) {

    auto numElementRegion = (uint32_t) (ceil((BRR - ULR) * (BRC - ULC) / 32.));
    _region = new uint32_t[numElementRegion];
    std::fill_n(_region, numElementRegion, valueFill);
  }
};

void creation(fc::FeatureCollection &mask) {
  std::vector<Region> vRegion;
  vRegion.emplace_back(1, 0, 0, 10, 3, 2863311530);    // 10101.....01010
  vRegion.emplace_back(2, 2, 3, 5, 7, 1431655765);    // 01010.....10101
  vRegion.emplace_back(3, 6, 10, 13, 13, 4294967295);    // 11111.....11111
  vRegion.emplace_back(4, 10, 5, 13, 8, 3435973836);    // 11001.....01100
  vRegion.emplace_back(5, 6, 5, 10, 10, 858993459);    // 00110.....00110

  for (auto region :vRegion) {
    mask.addFeature(region._id,
                    fc::BoundingBox(region._ULR,
                                    region._ULC,
                                    region._BRR,
                                    region._BRC),
                    region._region);
  }

  mask.setImageHeight(13);
  mask.setImageWidth(13);

  mask.preProcessing();

  for (auto region : vRegion) {
    delete[] region._region;
  }
}

void serialization(fc::FeatureCollection &mask, std::string &path) {
  mask.serialize(path);
}

void testFeatureCollection() {
  fc::FeatureCollection
      mask,
      maskCopy;

  std::string path = "test.serial";
  creation(mask);

  ASSERT_TRUE(mask.getFeatureFromPixel(0, 0)->getId() == 1);
  ASSERT_TRUE(mask.getFeatureFromPixel(4, 4)->getId() == 2);
  ASSERT_TRUE(mask.getFeatureFromPixel(8, 10)->getId() == 3);
  ASSERT_TRUE(mask.getFeatureFromPixel(11, 6)->getId() == 4);
  ASSERT_TRUE(mask.getFeatureFromPixel(9, 8)->getId() == 5);
  ASSERT_TRUE(mask.getFeatureFromPixel(0, 12) == nullptr);

  ASSERT_TRUE(mask.getFeatureFromId(2)->contains(2, 3));
  ASSERT_TRUE(mask.getFeatureFromId(2)->contains(2, 4));

  ASSERT_FALSE(mask.getFeatureFromId(2)->isInBitMask(2, 3));
  ASSERT_TRUE(mask.getFeatureFromId(2)->isInBitMask(2, 4));

  serialization(mask, path);
  maskCopy.deserialize(path);

  ASSERT_TRUE(mask == maskCopy);
};

void testMosaicCreation() {
  auto tileLoader = new fi::GrayscaleTiffTileLoader<uint8_t>("mask_mosaic.tif");
  auto *fi = new fi::FastImage<uint8_t>(tileLoader, 2);
  int rank = 8;

  uint32_t
      width = fi->getImageWidth(),
      height = fi->getImageHeight();

  FloodStrategy<uint8_t> fS(width, height);
  MaskAnalyser<uint8_t> ma(height, width);

  fi->configureAndRun();
  fi->requestAllTiles(true);
  while (fi->isGraphProcessingTiles()) {
    auto sharedView = fi->getAvailableViewBlocking();
    if (sharedView != nullptr) {
      auto view = sharedView->get();
      for (int32_t row = 0; row < view->getTileHeight(); ++row) {
        for (int32_t col = 0; col < view->getTileWidth(); ++col) {
          fS.insertPixel(row + view->getGlobalYOffset(),
                         col + view->getGlobalXOffset(),
                         view->getPixel(row, col));
        }
      }
      sharedView->releaseMemory();
    }
  }

  fS.label(rank);
  ASSERT_EQ(fS.getCurrentLabel(), 25);
  ma.initialize(fS.getCurrentLabel() - 1);
  ma.findBoundingBoxes(fS.getLabels());
  ma.setBitMask(fS.getLabels());
  ma.save("fc_mosaic.serial");
  fi->waitForGraphComplete();
  delete fi;
}

void testConnectivityAnalysis() {
  fc::FeatureCollection
    baseMask;

  ////////// Creation specific mask ////////////////////////
  std::vector<Region> vRegion;
  vRegion.emplace_back(0, 15, 15, 17, 17, 4026531840);
  vRegion.emplace_back(11, 18, 15, 19, 17, 3221225472);
  vRegion.emplace_back(12, 20, 15, 22, 17, 1610612736);
  vRegion.emplace_back(1, 15, 18, 17, 19, 3221225472);
  vRegion.emplace_back(2, 15, 20, 17, 22, 1610612736);
  vRegion.emplace_back(13, 23, 15, 25, 17, 2415919104);
  vRegion.emplace_back(18, 26, 15, 29, 17, 3087007744);
  vRegion.emplace_back(14, 31, 15, 33, 17, 4026531840);
  vRegion.emplace_back(15, 31, 18, 33, 19, 3221225472);
  vRegion.emplace_back(16, 31, 20, 33, 22, 2415919104);
  vRegion.emplace_back(3, 15, 23, 17, 25, 2415919104);
  vRegion.emplace_back(4, 15, 26, 17, 29, 3892314112);
  vRegion.emplace_back(5, 15, 31, 17, 33, 4026531840);
  vRegion.emplace_back(6, 18, 31, 19, 33, 3221225472);
  vRegion.emplace_back(7, 20, 31, 22, 33, 2415919104);
  vRegion.emplace_back(17, 31, 23, 33, 25, 1610612736);
  vRegion.emplace_back(19, 31, 26, 33, 29, 1543503872);
  vRegion.emplace_back(8, 23, 31, 25, 33, 1610612736);
  vRegion.emplace_back(9, 26, 31, 29, 33, 1946157056);
  vRegion.emplace_back(10, 31, 31, 33, 33, 4026531840);

  for (auto region :vRegion) {
    baseMask.addFeature(region._id,
                    fc::BoundingBox(region._ULR,
                                    region._ULC,
                                    region._BRR,
                                    region._BRC),
                    region._region);
  }

  baseMask.setImageHeight(48);
  baseMask.setImageWidth(48);

  baseMask.preProcessing();

  for (auto region : vRegion) {
    delete[] region._region;
  }

  baseMask.createBlackWhiteMask("maskConnectivity.tiff", 16);

  /////////// Creation FCs .////////////////////////
  fc::FeatureCollection
      fc4(new fi::GrayscaleTiffTileLoader<uint8_t>("maskConnectivity.tiff"), 4),
      fc8(new fi::GrayscaleTiffTileLoader<uint8_t>("maskConnectivity.tiff"), 8);

  ////////// Test /////////////////////

  ASSERT_EQ(fc4.getVectorFeatures().size(), 28);
  ASSERT_EQ(fc8.getVectorFeatures().size(), 20);
}

#endif //FASTIMAGE_TESTFEATURECOLLECTION_H
