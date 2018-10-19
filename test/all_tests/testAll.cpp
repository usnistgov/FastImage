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
// Created by anb22 on 2/1/18.
//

#ifndef FASTIMAGE_TESTGTEST_H
#define FASTIMAGE_TESTGTEST_H

#include <gtest/gtest.h>
#include "testFeatureCollection.h"
#include "testCache.h"
#include "testExceptions.h"
#include "testOrdered.h"
#include "testViewCounter.h"
#include "testTileLoader.h"
#include "testFastImageGlobal.h"
#include "testViewLoader.h"

void mosaicCreation() {
  auto
      *imageTab0 = new uint8_t[16 * 16](),
      *imageTab1 = new uint8_t[16 * 16],
      *maskTab = new uint8_t[16 * 16];

  for (int i = 0; i < 16; ++i) {
    if (i % 2 == 0) {
      std::fill_n(maskTab + i * 16, 16, 0);
    } else {
      std::fill_n(maskTab + i * 16, 16, 255);
    }
  }

  std::fill_n(imageTab0, 16 * 16, 0);
  std::fill_n(imageTab1, 16 * 16, 255);

  TIFF *img = TIFFOpen("mosaic.tif", "w");
  TIFF *mask = TIFFOpen("mask_mosaic.tif", "w");
  if (img != nullptr && mask != nullptr) {
    TIFFSetField(img, TIFFTAG_IMAGEWIDTH, 50);
    TIFFSetField(img, TIFFTAG_IMAGELENGTH, 48);
    TIFFSetField(img, TIFFTAG_TILEWIDTH, 16);
    TIFFSetField(img, TIFFTAG_TILELENGTH, 16);
    TIFFSetField(img, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(img, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
    TIFFSetField(img, TIFFTAG_ROWSPERSTRIP, 1);
    TIFFSetField(img, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(img, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(img, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    TIFFSetField(img, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    TIFFSetField(img, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

    TIFFSetField(mask, TIFFTAG_IMAGEWIDTH, 50);
    TIFFSetField(mask, TIFFTAG_IMAGELENGTH, 48);
    TIFFSetField(mask, TIFFTAG_TILEWIDTH, 16);
    TIFFSetField(mask, TIFFTAG_TILELENGTH, 16);
    TIFFSetField(mask, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(mask, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
    TIFFSetField(mask, TIFFTAG_ROWSPERSTRIP, 1);
    TIFFSetField(mask, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(mask, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(mask, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    TIFFSetField(mask, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    TIFFSetField(mask, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  } else {
    std::cerr << "The test files can't be created." << std::endl;
    exit(1);
  }

  uint32_t tile = 0;
  for (uint32_t row = 0; row < 3; ++row) {
    for (uint32_t col = 0; col < 4; ++col) {
      TIFFWriteEncodedTile(mask, tile, maskTab, (tsize_t) -1);
      if (row % 2 == 0) {
        if (col % 2 == 0) {
          TIFFWriteEncodedTile(img, tile, imageTab0, (tsize_t) -1);
        } else {
          TIFFWriteEncodedTile(img, tile, imageTab1, (tsize_t) -1);
        }
      } else {
        if (col % 2 == 0) {
          TIFFWriteEncodedTile(img, tile, imageTab1, (tsize_t) -1);
        } else {
          TIFFWriteEncodedTile(img, tile, imageTab0, (tsize_t) -1);
        }
      }
      ++tile;
    }
  }

  delete[]imageTab0;
  delete[]imageTab1;
  delete[]maskTab;

  TIFFClose(img);
  TIFFClose(mask);
}

TEST(TEST_FEATURE_COLLECTION, TEST_GLOBAL_FC) {
  ASSERT_NO_FATAL_FAILURE(testFeatureCollection());
  ASSERT_NO_FATAL_FAILURE(testMosaicCreation());
  ASSERT_NO_FATAL_FAILURE(testConnectivityAnalysis());
}

TEST(TEST_CACHE, NEW_CACHE) {
  ASSERT_NO_FATAL_FAILURE(createNewCache(0));
  ASSERT_NO_FATAL_FAILURE(createNewCache(10));
}

TEST(TEST_CACHE, INIT_CACHE) {
  ASSERT_NO_FATAL_FAILURE(createInitNewCache(0, 5, 5, 16, 16));
  ASSERT_NO_FATAL_FAILURE(createInitNewCache(0, 1, 5, 16, 16));
  ASSERT_NO_FATAL_FAILURE(createInitNewCache(100, 5, 5, 16, 16));
  ASSERT_NO_FATAL_FAILURE(createInitNewCache(10, 1, 5, 16, 16));
  ASSERT_NO_FATAL_FAILURE(createInitNewCache(1, 1, 5, 16, 16));
}

TEST(TEST_CACHE, GET_TILES) {
  ASSERT_NO_FATAL_FAILURE(getNewTiles(0, 5, 5, 16, 16));
  ASSERT_NO_FATAL_FAILURE(getNewTiles(0, 1, 5, 16, 16));
  ASSERT_NO_FATAL_FAILURE(getNewTiles(100, 5, 5, 16, 16));
  ASSERT_NO_FATAL_FAILURE(getNewTiles(10, 1, 5, 16, 16));
  ASSERT_NO_FATAL_FAILURE(getNewTiles(10, 1, 5, 16, 16));
}

TEST(TEST_VIEW_LOADER, TEST_VIEW_REQUEST_DATA) {
  ASSERT_NO_FATAL_FAILURE(testViewRequestData());
}

TEST(TEST_VIEW_LOADER, TEST_VIEW_LOADING) {
  ASSERT_NO_FATAL_FAILURE(testViewLoaderTileGhostUL());
  ASSERT_NO_FATAL_FAILURE(testViewLoaderTileGhostBR());
}

TEST(TEST_TILE_LOADER, TEST_TILE_LOADING) {
  ASSERT_NO_FATAL_FAILURE(testTileLoading());
}

TEST(TEST_VIEW_COUNTER, TEST_VIEW_CREATION) {
  ASSERT_NO_FATAL_FAILURE(testViewCounterNoRadius());
  ASSERT_NO_FATAL_FAILURE(testViewCounterRadiusUL());
  ASSERT_NO_FATAL_FAILURE(testViewCounterRadiusBR());
}

TEST(TEST_GLOBAL, TEST_PROCESS) {
  ASSERT_NO_FATAL_FAILURE(testWholeImage());
  ASSERT_NO_FATAL_FAILURE(testPartImage());
  ASSERT_NO_FATAL_FAILURE(testSingleTile());
}

TEST(TEST_EXCEPTION, TEST_FAILURE) {
  ASSERT_NO_FATAL_FAILURE(testOutOfBounds());
  ASSERT_NO_FATAL_FAILURE(testCacheOutOfBounds());
}

TEST(TEST_ORDERING, TEST_ORDERING) {
  ASSERT_TRUE(testOrdered());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  mosaicCreation();

  int ret = RUN_ALL_TESTS();

  return ret;

}
#endif //FASTIMAGE_TESTGTEST_H
