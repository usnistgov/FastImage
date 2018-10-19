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

/// @file FeatureCollection.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  8/9/17
/// @brief Mask representation, used with FastImage

#ifndef FEATURECOLLECTION_H
#define FEATURECOLLECTION_H

#include <sstream>
#include "FastImage/FeatureCollection/Tasks/BlobMerger.h"
#include "FastImage/FeatureCollection/tools/AABBTree.h"
#include "Feature.h"
#include "BoundingBox.h"
#include "FastImage/exception/FastImageException.h"

#include "tools/UnionFind.h"
#include "Tasks/ViewAnalyser.h"

namespace fc {
/// \namespace fc FeatureCollection namespace

/**
  * @class FeatureCollection FeatureCollection.h <FastImage/FeatureCollection/FeatureCollection.h>
  *
  * @brief Representation of a mask.
  *
  * @details FastImage way to represent and use a mask. Instead to have a pixel
  * centric approach as the mask, it has been chosen to use a Feature centric
  * approach. A Feature is a set of connected pixel representing a separated
  * region of pixels.
  *
  * An AABB tree is used to provide a quick lookup from pixel to feature.
  *
  * Convenient methods have been made to create a Feature collection from an image,
  * and the inverse operation.
  **/
class FeatureCollection {
 public:
  using iteratorFeatures = std::vector<Feature>::iterator;

  /// \brief Default Feature Collection constructor
  FeatureCollection() : _imageWidth(0), _imageHeight(0) {}

  /// \brief Open and deserialized a FC file
  /// \param pathFeatureCollection Path to the FC file
  explicit FeatureCollection(const std::string &pathFeatureCollection)
      : _imageWidth(0), _imageHeight(0) {
    this->deserialize(pathFeatureCollection);
  }

  /// \brief Create a feature collection from a mask
  /// \tparam T File type
  /// \param tileLoader Tile loader to access to the file
  /// \param rank Connectivity rank [default 8]
  /// \param background Background value [default 0]
  /// \param numberOfThreadsParallel Number of thread use
  /// [default std::thread::hardware_concurrency()]
  /// \param numberOfViewParallel Number of views used in parallel
  /// [default 4 * std::thread::hardware_concurrency()]
  /// \note The Tile Loader that is sent into the FeatureCollection
  /// will be deleted.
  template<class UserType>
  explicit FeatureCollection(fi::ATileLoader<UserType> *tileLoader,
                             uint8_t rank = 8,
                             UserType background = 0,
                             uint32_t numberOfThreadsParallel =
                             std::thread::hardware_concurrency(),
                             uint32_t numberOfViewParallel = 4
                                 * std::thread::hardware_concurrency())
      : _imageWidth(0), _imageHeight(0) {
    fi::FastImage<UserType> *
        fi;              //< Fast Image object going through the mask

    uint32_t
        imageHeight,     //< Image Height
        imageWidth;      //< Image Width

    htgs::TaskGraphConf<htgs::MemoryData<fi::View<UserType>>, ListBlobs>
        *analyseGraph;  //< Analyse graph

    htgs::TaskGraphRuntime
        *analyseRuntime; //< Analyse runtime

    assert(rank == 8 || rank == 4);
    try {
      //Create the Fast image and get the info from it
      //The radius of 1 is used to make the link between the tiles
      fi = new fi::FastImage<UserType>(tileLoader, 1);
      fi->getFastImageOptions()->setNumberOfViewParallel(numberOfViewParallel);
      fi->configureAndRun();
      imageHeight = fi->getImageHeight();
      imageWidth = fi->getImageWidth();

      //Set the number of threads if incorrect
      if (numberOfThreadsParallel == 0) { numberOfThreadsParallel = 8; }

    } catch (const fi::FastImageException &e) {
      std::cerr << e.what() << std::endl;
      exit(EXIT_FAILURE);
    } catch (const std::exception &e) {
      std::cerr << e.what() << std::endl;
      exit(EXIT_FAILURE);
    }

    // Create the analyse graph
    analyseGraph = new htgs::TaskGraphConf<htgs::MemoryData<fi::View<UserType>>,
                                           ListBlobs>;
    auto viewAnalyseTask = new ViewAnalyser<UserType>(numberOfThreadsParallel,
                                                      fi,
                                                      rank,
                                                      background);
    auto fileCreation = new BlobMerger(imageHeight,
                                       imageWidth,
                                       fi->getNumberTilesHeight()
                                           * fi->getNumberTilesWidth());

    analyseGraph->setGraphConsumerTask(viewAnalyseTask);
    analyseGraph->addEdge(viewAnalyseTask, fileCreation);
    analyseGraph->addGraphProducerTask(fileCreation);
    analyseRuntime = new htgs::TaskGraphRuntime(analyseGraph);
    // Launch the analyse graph
    analyseRuntime->executeRuntime();

    try {
      //Request all tiles from the FC
      fi->requestAllTiles(true);
      while (fi->isGraphProcessingTiles()) {
        auto pView = fi->getAvailableViewBlocking();
        if (pView != nullptr) {
          //Feed the graph with the views
          analyseGraph->produceData(pView);
        }
      }
    } catch (const fi::FastImageException &e) {
      std::cerr << e.what() << std::endl;
      exit(EXIT_FAILURE);
    } catch (const std::exception &e) {
      std::cerr << e.what() << std::endl;
      exit(EXIT_FAILURE);
    }

    analyseGraph->finishedProducingData();

    auto blob = analyseGraph->consumeData();
    this->createFCFromListBlobs(blob.get(),
                                fi->getImageHeight(0),
                                fi->getImageWidth(0));

    // Wait for the analyse graph to finish processing tiles to make the FC
    // available
    analyseRuntime->waitForRuntime();
    fi->waitForGraphComplete();

    // Delete HTGS graphs
    delete (fi);
    delete (analyseRuntime);

  }

  /// \brief Default FeatureCollection destructor
  virtual ~FeatureCollection() {
    for (const auto &feature : _vectorFeatures) {
      if (feature.getBitMask() != nullptr)
        delete[] feature.getBitMask();
    }
  }

  /// \brief Get Image Width
  /// \return Image Width
  uint32_t getImageWidth() const { return _imageWidth; }

  /// \brief Get Image Height
  /// \return Image height
  uint32_t getImageHeight() const { return _imageHeight; }

  /// \brief Set Image Width
  /// \param imageWidth New image width
  void setImageWidth(uint32_t imageWidth) { _imageWidth = imageWidth; }

  /// \brief Set Image height
  /// \param imageHeight New image height
  void setImageHeight(uint32_t imageHeight) { _imageHeight = imageHeight; }

  /// \brief Get a feature from a pixel
  /// \param row row pixel
  /// \param col column pixel
  /// \return Feature
  Feature *getFeatureFromPixel(uint32_t row, uint32_t col) {
    for (auto pFeature : _tree.objectsContain(Vector2<double>((double) col,
                                                              (double) row))) {
      if (pFeature->isInBitMask(row, col)) {
        return pFeature;
      }
    }
    return nullptr;
  }

  /// \brief Get a feature from an id
  /// \param id Feature Id looking for
  /// \return Feature adress if found, else nullptr
  Feature *getFeatureFromId(uint32_t id) {
    Feature *f = nullptr;
    bool found = false;
    for (auto it = _vectorFeatures.begin();
         it != _vectorFeatures.end() && !found; ++it) {
      if (it->getId() == id) {
        found = true;
        f = &(*it);
      }
    }
    return f;
  }

  /// \brief Preprocessing function to call to build an AABB tree as soon as
  /// all the feature have been added
  void preProcessing() {
    if (this->getImageWidth() == 0 || this->getImageHeight() == 0) {
      std::stringstream message;
      message
          << "Feature ERROR: The image size is not the same as the feature "
             "collection size.";
      std::string m = message.str();
      throw (fi::FastImageException(m));
    }
    _tree.preprocess(&_vectorFeatures);
  }

  /// \brief Create and add a feature to the feature collection
  /// \param id Feature Id
  /// \param boundingBox Feature's bounding box
  /// \param bitMask Feature's bitmask
  void addFeature(uint32_t id,
                  const BoundingBox &boundingBox,
                  uint32_t *bitMask) {
    this->_vectorFeatures.emplace_back(id, boundingBox, bitMask);
  }

  /// \brief Get the vector of features from the Feature collection
  /// \return The vector of features from the Feature collection
  const std::vector<Feature> &getVectorFeatures() const {
    return _vectorFeatures;
  }

  /// \brief Get first feature iterator
  /// \return First feature iterator
  iteratorFeatures begin() {
    return _vectorFeatures.begin();
  }

  /// \brief Get last feature iterator
  /// \return Last feature iterator
  iteratorFeatures end() {
    return _vectorFeatures.end();
  }

  /// \brief Serialize the feature collection into a binary file
  /// \param path Path to the binary file
  void serialize(const std::string &path) {
    try {
      std::ofstream outfile(path, std::ofstream::binary);
      if (outfile.is_open()) {
        outfile << _imageHeight << " " << _imageWidth << " ";
        outfile << _vectorFeatures.size() << " ";
        for (auto feature:_vectorFeatures) {
          feature.serializeFeature(outfile);
        }
        outfile.close();
      } else {
        std::stringstream message;
        message << "Feature Collection ERROR: The Feature collection at path \""
                << path << "\" can't be saved.";
        std::string m = message.str();
        throw (fi::FastImageException(m));
      }
    } catch (const std::exception &e) {
      std::stringstream message;
      message << "Unhandled exception: " << e.what();
      std::string m = message.str();
      throw (fi::FastImageException(m));
    }
  }

  /// \brief Deserialize a feature collection from a file
  /// \param path Path to the binary file
  void deserialize(const std::string &path) {
    try {
      std::ifstream inFile(path, std::ifstream::binary);
      if (inFile.is_open()) {
        std::vector<Feature>::size_type sizeVector;

        inFile >> _imageHeight;
        inFile >> _imageWidth;
        inFile >> sizeVector;

        for (uint32_t i = 0; i < sizeVector; ++i) {
          this->_vectorFeatures.push_back(Feature::deserializeFeature(inFile));
        };

        this->preProcessing();

        inFile.close();
      } else {
        std::stringstream message;
        message << "Feature Collection ERROR: The Feature collection at path \""
                << path << "\" can't be opened.";
        std::string m = message.str();
        throw (fi::FastImageException(m));
      }
    } catch (const std::exception &e) {
      std::stringstream message;
      message << "Unhandled exception: " << e.what();
      std::string m = message.str();
      throw (fi::FastImageException(m));
    }
  }

  /// \brief Output stream operator
  /// \param os Output stream
  /// \param mask Feature Collection to print
  /// \return Output stream with the data printed
  friend std::ostream &operator<<(std::ostream &os,
                                  const FeatureCollection &mask) {
    os << "Image Height: " << mask.getImageHeight() << " Image width: "
       << mask.getImageWidth() << std::endl;
    for (const auto &feature : mask._vectorFeatures) {
      os << feature << std::endl << std::flush;
    }
    return os;
  }

  /// \brief Equality operator
  /// \param fc Feature collection to compare with
  /// \return True if equal, else False
  bool operator==(FeatureCollection &fc) {
    bool answer = true;
    auto b = fc._vectorFeatures.begin();
    auto e = fc._vectorFeatures.end();

    if (this->_vectorFeatures.size() == fc._vectorFeatures.size()) {
      for(auto feature = this->_vectorFeatures.begin(); feature != this->_vectorFeatures.end() && answer; ++feature){
        if(std::find(b,e, *feature) == e){
          answer = false;
        }
      }

    } else {
      answer = false;
    }

    return answer;
  }

  /// \brief Create a tiled tiff mask, where the pixels are the feature id.
  /// \param pathLabeledMask Path to save the mask.
  /// \param tileSize Size of tile in the tiff image
  void createLabeledMask(const std::string &pathLabeledMask,
                         const uint32_t tileSize = 1024) {
    if ((tileSize & (tileSize - 1)) != 0) {
      std::stringstream message;
      message
          << "Feature Collection ERROR: The tiling asked is not a power of 2.";
      std::string m = message.str();
      throw (fi::FastImageException(m));
    }
    // Get image size
    auto
        imageWidth = this->getImageWidth(),
        imageHeight = this->getImageHeight();

    // Create two tiles pointer: one empty, one to be filled
    std::vector<uint32_t >
        *emptyTile = new std::vector<uint32_t>(tileSize * tileSize, 0),
        *currentTile = nullptr;

    // Create a pair to create a coordinate (row/col)
    std::pair<uint32_t, uint32_t>
        currentCoordinates;

    // Create the tiff file
    TIFF
        *tif = TIFFOpen(pathLabeledMask.c_str(), "w");

    uint32_t
        upperLeftRowFeature = 0,
        upperLeftColFeature = 0,
        bottomRightRowFeature = 0,
        bottomRightColFeature = 0,
        minRowTile = 0,
        minColTile = 0,
        upperLeftRow = 0,
        upperLeftCol = 0,
        bottomRightRow = 0,
        bottomRightCol = 0,
        indexMinTileRow = 0,
        indexMaxTileRow = 0,
        indexMinTileCol = 0,
        indexMaxTileCol = 0;

    // Create a map of coordinate -> Tile
    std::map<std::pair<uint32_t, uint32_t>, std::vector<uint32_t> *>
        loadedTiles;

    if (tif != nullptr) {
      TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, imageWidth);
      TIFFSetField(tif, TIFFTAG_IMAGELENGTH, imageHeight);
      TIFFSetField(tif, TIFFTAG_TILELENGTH, tileSize);
      TIFFSetField(tif, TIFFTAG_TILEWIDTH, tileSize);
      TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8 * sizeof(uint32_t));
      TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
      TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
      TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
      TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
      TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
      TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
      TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

      // For every feature in the feature collection
      for (const auto &feature : this->getVectorFeatures()) {
        // Get the bounding box
        upperLeftRowFeature = feature.getBoundingBox().getUpperLeftRow();
        upperLeftColFeature = feature.getBoundingBox().getUpperLeftCol();
        bottomRightRowFeature = feature.getBoundingBox().getBottomRightRow();
        bottomRightColFeature = feature.getBoundingBox().getBottomRightCol();

        indexMinTileRow = upperLeftRowFeature / tileSize;
        indexMinTileCol = upperLeftColFeature / tileSize;

        indexMaxTileRow = (bottomRightRowFeature - 1) / tileSize;
        indexMaxTileCol = (bottomRightColFeature - 1) / tileSize;

        // For every view in the bounding box
        for (uint32_t indexRow = indexMinTileRow; indexRow <= indexMaxTileRow;
             ++indexRow) {
          for (uint32_t indexCol = indexMinTileCol; indexCol <= indexMaxTileCol;
               ++indexCol) {

            // Get the tile, from the map or create one
            currentCoordinates =
                std::pair<uint32_t, uint32_t>(indexRow, indexCol);
            if (loadedTiles.find(currentCoordinates) == loadedTiles.end()) {
              loadedTiles[currentCoordinates] =
                  new std::vector<uint32_t>(tileSize * tileSize, 0);
            }
            currentTile = loadedTiles[currentCoordinates];

            // Get the indexes used for the computation
            minRowTile = indexRow * tileSize;
            minColTile = indexCol * tileSize;

            upperLeftRow = std::max(minRowTile, upperLeftRowFeature);
            upperLeftCol = std::max(minColTile, upperLeftColFeature);

            bottomRightRow =
                std::min(bottomRightRowFeature, (indexRow + 1) * tileSize);
            bottomRightCol =
                std::min(bottomRightColFeature, (indexCol + 1) * tileSize);

            // For every pixel in the tile
            for (uint32_t rowTileFeature = upperLeftRow;
                 rowTileFeature < bottomRightRow; ++rowTileFeature) {
              for (uint32_t colTileFeature = upperLeftCol;
                   colTileFeature < bottomRightCol; ++colTileFeature) {

                // If the pixel is in the feature
                if (feature.isInBitMask(rowTileFeature, colTileFeature)) {
                  currentTile->at((rowTileFeature - minRowTile) * tileSize
                                      + (colTileFeature - minColTile))
                      = (uint32_t) (feature.getId() + 1);
                }
              }
            }
          }
        }
      }

      indexMaxTileRow = (this->getImageHeight() - 1) / tileSize;
      indexMaxTileCol = (this->getImageWidth() - 1) / tileSize;

      // For every tile in the image
      for (uint32_t indexRow = 0; indexRow <= indexMaxTileRow; indexRow++) {
        for (uint32_t indexCol = 0; indexCol <= indexMaxTileCol; indexCol++) {

          // Get the tile from it coordinates in the map
          currentCoordinates =
              std::pair<uint32_t, uint32_t>(indexRow, indexCol);
          // If it doesn't exist write the blank tile
          if (loadedTiles.find(currentCoordinates) == loadedTiles.end()) {
            TIFFWriteTile(tif,
                          emptyTile,
                          indexCol * tileSize,
                          indexRow * tileSize,
                          0,
                          0);
          } else {
            // Write the filled tile
            TIFFWriteTile(tif,
                          loadedTiles[currentCoordinates]->data(),
                          indexCol * tileSize,
                          indexRow * tileSize,
                          0,
                          0);
          }

        }
      }
      // Close the tif
      TIFFClose(tif);
    } else {
      std::cerr << "The File " << pathLabeledMask << " can't be opened."
                << std::endl;
      exit(1);
    }

    delete emptyTile;
    for (auto &tile : loadedTiles) {
      delete tile.second;
    }
  }

  /// \brief Create a tiled tiff mask, where the pixels are 1.
  /// \param pathLabeledMask Path to save the mask.
  /// \param tileSize Size of tile in the tiff image
  void createBlackWhiteMask(const std::string &pathLabeledMask,
                            const uint32_t tileSize = 1024) {
    if ((tileSize & (tileSize - 1)) != 0) {
      std::stringstream message;
      message
          << "Feature Collection ERROR: The tiling asked is not a power of 2.";
      std::string m = message.str();
      throw (fi::FastImageException(m));
    }
    // Get image size
    auto
        imageWidth = this->getImageWidth(),
        imageHeight = this->getImageHeight();

    // Create two tiles pointer: one empty, one to be filled
    std::vector<uint8_t>
        *emptyTile = new std::vector<uint8_t>(tileSize * tileSize, 0),
        *currentTile = nullptr;

    // Create a pair to create a coordinate (row/col)
    std::pair<uint32_t, uint32_t>
        currentCoordinates;

    // Create the tiff file
    TIFF
        *tif = TIFFOpen(pathLabeledMask.c_str(), "w");

    uint32_t
        upperLeftRowFeature = 0,
        upperLeftColFeature = 0,
        bottomRightRowFeature = 0,
        bottomRightColFeature = 0,
        minRowTile = 0,
        minColTile = 0,
        upperLeftRow = 0,
        upperLeftCol = 0,
        bottomRightRow = 0,
        bottomRightCol = 0,
        indexMinTileRow = 0,
        indexMaxTileRow = 0,
        indexMinTileCol = 0,
        indexMaxTileCol = 0;

    // Create a map of coordinate -> Tile
    std::map<std::pair<uint32_t, uint32_t>, std::vector<uint8_t> *>
        loadedTiles;

    if (tif != nullptr) {
      TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, imageWidth);
      TIFFSetField(tif, TIFFTAG_IMAGELENGTH, imageHeight);
      TIFFSetField(tif, TIFFTAG_TILELENGTH, tileSize);
      TIFFSetField(tif, TIFFTAG_TILEWIDTH, tileSize);
      TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8 * sizeof(uint8_t));
      TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
      TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
      TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
      TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
      TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
      TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
      TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

      // For every feature in the feature collection
      for (const auto &feature : this->getVectorFeatures()) {
        // Get the bounding box
        upperLeftRowFeature = feature.getBoundingBox().getUpperLeftRow();
        upperLeftColFeature = feature.getBoundingBox().getUpperLeftCol();
        bottomRightRowFeature = feature.getBoundingBox().getBottomRightRow();
        bottomRightColFeature = feature.getBoundingBox().getBottomRightCol();

        indexMinTileRow = upperLeftRowFeature / tileSize;
        indexMinTileCol = upperLeftColFeature / tileSize;

        indexMaxTileRow = (bottomRightRowFeature - 1) / tileSize;
        indexMaxTileCol = (bottomRightColFeature - 1) / tileSize;

        // For every view in the bounding box
        for (uint32_t indexRow = indexMinTileRow; indexRow <= indexMaxTileRow;
             ++indexRow) {
          for (uint32_t indexCol = indexMinTileCol; indexCol <= indexMaxTileCol;
               ++indexCol) {

            // Get the tile, from the map or create one
            currentCoordinates =
                std::pair<uint32_t, uint32_t>(indexRow, indexCol);
            if (loadedTiles.find(currentCoordinates) == loadedTiles.end()) {
              loadedTiles[currentCoordinates] =
                  new std::vector<uint8_t>(tileSize * tileSize, 0);
            }
            currentTile = loadedTiles[currentCoordinates];

            // Get the indexes used for the computation
            minRowTile = indexRow * tileSize;
            minColTile = indexCol * tileSize;

            upperLeftRow = std::max(minRowTile, upperLeftRowFeature);
            upperLeftCol = std::max(minColTile, upperLeftColFeature);

            bottomRightRow =
                std::min(bottomRightRowFeature, (indexRow + 1) * tileSize);
            bottomRightCol =
                std::min(bottomRightColFeature, (indexCol + 1) * tileSize);

            // For every pixel in the tile
            for (uint32_t rowTileFeature = upperLeftRow;
                 rowTileFeature < bottomRightRow; ++rowTileFeature) {
              for (uint32_t colTileFeature = upperLeftCol;
                   colTileFeature < bottomRightCol; ++colTileFeature) {

                // If the pixel is in the feature
                if (feature.isInBitMask(rowTileFeature, colTileFeature)) {
                  currentTile->at((rowTileFeature - minRowTile) * tileSize
                                      + (colTileFeature - minColTile)) = 1;
                }
              }
            }
          }
        }
      }

      indexMaxTileRow = (this->getImageHeight() - 1) / tileSize;
      indexMaxTileCol = (this->getImageWidth() - 1) / tileSize;

      // For every tile in the image
      for (uint32_t indexRow = 0; indexRow <= indexMaxTileRow; indexRow++) {
        for (uint32_t indexCol = 0; indexCol <= indexMaxTileCol; indexCol++) {

          // Get the tile from it coordinates in the map
          currentCoordinates =
              std::pair<uint32_t, uint32_t>(indexRow, indexCol);
          // If it doesn't exist write the blank tile
          if (loadedTiles.find(currentCoordinates) == loadedTiles.end()) {
            TIFFWriteTile(tif,
                          emptyTile,
                          indexCol * tileSize,
                          indexRow * tileSize,
                          0,
                          0);
          } else {
            // Write the filled tile
            TIFFWriteTile(tif,
                          (loadedTiles[currentCoordinates])->data(),
                          indexCol * tileSize,
                          indexRow * tileSize,
                          0,
                          0);
          }

        }
      }
      // Close the tif
      TIFFClose(tif);
    } else {
      std::cerr << "The File " << pathLabeledMask << " can't be opened."
                << std::endl;
      exit(1);
    }

    delete emptyTile;
    for (auto &tile : loadedTiles) {
      delete tile.second;
    }
  }

 private:
  void createFCFromListBlobs(const ListBlobs *listBlobs,
                             uint32_t imageHeight,
                             uint32_t imageWidth) {

    uint32_t
        idFeature = 0,
        rowMin = 0,
        colMin = 0,
        rowMax = 0,
        colMax = 0,
        ulRowL = 0,
        ulColL = 0,
        wordPosition = 0,
        bitPosition = 0,
        absolutePosition = 0;

    uint32_t *bitMask = nullptr;

    this->setImageHeight(imageHeight);
    this->setImageWidth(imageWidth);

    //Iterate over the blobs
    for (auto blob : listBlobs->_blobs) {
      rowMin = (uint32_t) blob->getRowMin();
      rowMax = (uint32_t) blob->getRowMax();
      colMin = (uint32_t) blob->getColMin();
      colMax = (uint32_t) blob->getColMax();

      // Create the bounding box
      fc::BoundingBox bB(rowMin, colMin, rowMax, colMax);

      bitMask = new uint32_t[(uint32_t) ceil(
          (bB.getHeight() * bB.getWidth()) / 32.)]();
      // For every pixel in the bit mask
      for (uint32_t row = rowMin; row < rowMax; ++row) {
        for (uint32_t col = colMin; col < colMax; ++col) {
          // Test if the pixel is in the current feature
          if (blob->isPixelinFeature(row, col)) {
            // Add it to the bit mask
            ulRowL = row - bB.getUpperLeftRow();
            ulColL = col - bB.getUpperLeftCol();
            absolutePosition = ulRowL * bB.getWidth() + ulColL;
            wordPosition = absolutePosition >> (uint32_t) 5;
            bitPosition =
                (uint32_t) abs(32 - ((int32_t) absolutePosition
                    - ((int32_t) (wordPosition << (uint32_t) 5))));
            bitMask[wordPosition] = bitMask[wordPosition]
                | ((uint32_t) 1 << (bitPosition - (uint32_t) 1));
          }
        }
      }

      //Add the feature to the FC
      this->addFeature(idFeature, bB, bitMask);
      ++idFeature;
      // Delete the bitmask
      delete[](bitMask);
    }

    // Preprocess the FC
    this->preProcessing();
  }

  std::vector<Feature>
      _vectorFeatures{};    ///< Vector of features

  uint32_t
      _imageWidth,        ///< Image width
      _imageHeight;       ///< Image height

  AABBTree<Feature>
      _tree;              ///< AABB tree to quick lookup from pixel to feature
};
}

#endif //FEATURECOLLECTION_H
