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

#ifndef FASTIMAGE_TESTCACHE_H
#define FASTIMAGE_TESTCACHE_H

#include <gtest/gtest.h>
#include <cmath>
#include "FastImage/object/FigCache.h"

void createNewCache(uint32_t numTileCache) {
  fi::FigCache<int> cache(numTileCache);
  ASSERT_EQ(cache.getHit(), 0);
  ASSERT_EQ(cache.getMiss(), 0);
}

void createInitNewCache(uint32_t numTileCache,
                        uint32_t numTilesHeight,
                        uint32_t numTilesWidth,
                        uint32_t tileHeight,
                        uint32_t tileWidth) {
  fi::FigCache<int> cache(numTileCache);
  cache.initCache(numTilesHeight, numTilesWidth, tileHeight, tileWidth);

  if (numTileCache == 0) { numTileCache = 2 * numTilesWidth; }
  if (numTilesHeight * numTilesWidth < numTileCache) {
    ASSERT_EQ(cache.getNbTilesCache(), numTilesHeight * numTilesWidth);
  } else {
    ASSERT_EQ(cache.getNbTilesCache(), numTileCache);
  }
  ASSERT_EQ(cache.getNbTilesCache(), cache.getPool().size());
  auto mapCache = cache.getMapCache();
  for (uint32_t row = 0; row < numTilesHeight; ++row) {
    for (uint32_t col = 0; col < numTilesWidth; ++col) {
      ASSERT_EQ(mapCache[row][col], nullptr);
    }
  }
  ASSERT_EQ(cache.getLru().size(), 0);
}

void getNewTiles(uint32_t numTileCache,
                 uint32_t numTilesHeight,
                 uint32_t numTilesWidth,
                 uint32_t tileHeight,
                 uint32_t tileWidth) {

  fi::CachedTile<int> *tile;
  fi::FigCache<int> cache(numTileCache);
  cache.initCache(numTilesHeight, numTilesWidth, tileHeight, tileWidth);

  uint32_t
      nbTilesCache = cache.getNbTilesCache();

  ASSERT_THROW(cache.getLockedTile(numTilesHeight + 1, 0),
               fi::FastImageException);
  ASSERT_THROW(cache.getLockedTile(numTilesHeight + 1, numTilesWidth + 1),
               fi::FastImageException);
  ASSERT_THROW(cache.getLockedTile(0, numTilesWidth + 1),
               fi::FastImageException);

  tile = cache.getLockedTile(0, 0);
  ASSERT_EQ(tile->isNewTile(), true);
  tile->setNewTile(false);
  tile->unlock();
  ASSERT_EQ(cache.getPool().size(), nbTilesCache - 1);
  ASSERT_EQ(cache.getLru().front(), cache.getMapCache()[0][0]);

  tile = cache.getLockedTile(0, 0);
  ASSERT_EQ(tile->isNewTile(), false);
  tile->unlock();
  ASSERT_EQ(cache.getPool().size(), nbTilesCache - 1);
  ASSERT_EQ(cache.getLru().front(), cache.getMapCache()[0][0]);

  tile = cache.getLockedTile(numTilesHeight - 1, numTilesWidth - 1);
  ASSERT_EQ(tile->isNewTile(), true);
  tile->unlock();

  ASSERT_EQ(cache.getPool().size(),
            std::max((int32_t) nbTilesCache - 2, (int32_t) 0));
  ASSERT_EQ(cache.getLru().front(),
            cache.getMapCache()[numTilesHeight - 1][numTilesWidth - 1]);

  for (uint32_t alreadyUsedTiles = 1; alreadyUsedTiles < nbTilesCache;
       ++alreadyUsedTiles) {
    tile = cache.getLockedTile(alreadyUsedTiles / numTilesWidth,
                               alreadyUsedTiles % numTilesWidth);
    ASSERT_EQ(tile->isNewTile(), true);
    tile->setNewTile(false);
    tile->unlock();
  }

  ASSERT_EQ(cache.getPool().size(), 0);

  tile = cache.getLockedTile(0, 0);
  ASSERT_EQ(tile->isNewTile(), nbTilesCache != numTilesHeight * numTilesWidth);
  tile->setNewTile(false);
  tile->unlock();
  ASSERT_EQ(cache.getPool().size(), 0);
  ASSERT_EQ(cache.getLru().front(), cache.getMapCache()[0][0]);
}

#endif //FASTIMAGE_TESTCACHE_H
