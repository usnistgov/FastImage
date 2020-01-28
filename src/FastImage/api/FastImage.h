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

/// @file FastImage.h
/// @author Alexandre Bardakoff - Timothy Blattner
/// @date  7/27/17
/// @brief Main object to manage a picture


#ifndef FASTIMAGE_H
#define FASTIMAGE_H

#include <utility>
#include <cstdint>
#include <algorithm>
#include <cmath>

#include <htgs/api/TaskGraphConf.hpp>
#include <htgs/api/TaskGraphRuntime.hpp>
#include <htgs/api/ExecutionPipeline.hpp>
#include <htgs/api/TGTask.hpp>

#include "ATileLoader.h"
#include "FastImage/tasks/ViewLoader.h"
#include "FastImage/tasks/ViewCounter.h"
#include "../memory/ViewAllocator.h"
#include "../memory/VariableMemoryManager.h"
#include "../rules/DistributePyramidRule.h"
#include "../object/FigCache.h"
#include "../object/Traversal.h"
#include "../FeatureCollection/Feature.h"
#include "../exception/FastImageException.h"

namespace fi {
/// \namespace fi FastImage namespace

/**
 * @class FastImage FastImage.h <FastImage/api/FastImage.h>
 * @brief Main api object to access the targeted image.
 * @details The FastImage object will be use to access the image. It is
 * templatized, with the format the user want the image's pixel. It is an HTGS
 * graph working with a cache to avoid too much IO from the disk. All the view
 * requests are sent to a View Loader, which transfer requests for specific
 * tiles to the tiles loader (file format dependant). Then pieces of the view
 * will be sent to a View counter to finalized the view. Once ready, the final
 * view is made available.
 *
 * It will use an ATileLoader to access the underlying image.
 *
 * @code
 * auto *tileLoader = new fi::GrayscaleTiffTileLoader<uint32_t>(pathImage, 10);
 * auto *fi = new fi::FastImage<uint32_t>(tileLoader, 0);
 * @endcode
 *
 * Once opened, it can be configured more precisely thanks to it options.
 *
 * @code
 * fi->getFastImageOptions()->setPreserveOrder(preserveOrder);
 * fi->getFastImageOptions()->setFinishRequestingViews(finishRequestingViews);
 * fi->getFastImageOptions()->setNumberOfViewParallel(numberOfViewParallel);
 * fi->getFastImageOptions()->setNumberOfTilesToCache(numberOfTilesToCache);
 * fi->getFastImageOptions()->setNumberOfTileLoader(numberOfTileLoader);
 * fi->getFastImageOptions()->setTraversalType(traversalType);
 * fi->getFastImageOptions()->setFillingType(fillingType);
 * fi->getFastImageOptions()->setNbReleasePyramid(pyramidLvl, nbRelease);
 * @endcode
 *
 * When the configuration is done, the graph can be executed:
 *
 * @code
 * fi->configureAndRun();
 * @endcode
 *
 * The user can request some view from FastImage:
 *
 * @code
 * fi->requestTile(...);
 * fi->requestFeature(...);
 * fi->requestAllTiles(...);
 * @endcode
 *
 * NOTE: The boolean parameter indicate to the system that no more views will be
 * requested.
 *
 * Once the views are requested they can be acquired and can be used:
 *
 * @code
 * while (fi->isGraphProcessingTiles()) {
 *   auto pView = fi->getAvailableViewBlocking();
 *   if (pView != nullptr) {
 *   ...
 *   pView->releaseMemory();
 *   }
 * }
 * @endcode
 *
 * If not already done, we have to tell to the system no more tiles will be
 * requested
 *
 * @code
 * fi->finishedRequestingTiles();
 * @endcode
 *
 * Wait for fast image to be done (not entirely necessary, but makes it safe for cleanup)
 * @code
 * fi->waitForGraphComplete();
 * @endcode
 *
 * Then the system can release all memory used by fast image (including the tile
 * loader)
 * @code
 * delete fi;
 * @endcode
 *
 * @tparam UserType Pixel Type asked by the end user
 */
template<typename UserType>
class FastImage {

  /**
 * @class fi::FastImage::Options FastImage.h <FastImage/api/FastImage.h>
 * @brief Options to customise FastImage.
 */
  class Options {
   public:
    /// \brief FastImage Options constructor
    /// \details Sets the default values for Fast Image as follow:
    /// @code
    ///  finishRequestingViews = false;
    ///  preserveOrder = false;
    ///  numberOfViewParallel = 1;
    ///  numberOfTilesToCache = 0;
    ///  numberOfTileLoader = 1;
    ///  traversalType = TraversalType::SNAKE;
    ///  fillingType = FillingType::FILL;
    ///  nbReleasePyramid = 1; // 1 for each level
    /// @endcode
    ///
    /// \param nbPyramidLevel Number of pyramid level
    explicit Options(uint32_t nbPyramidLevel) {
      _nbReleasePyramid = std::vector<uint32_t>(nbPyramidLevel, 1);
    }

    /// \brief Get if is finish sent
    /// \return Finish sent boolean
    bool isFinishedRequestingViews() const { return _finishRequestingViews; }

    /// \brief Get if output order is the same as the requested order
    /// \return
    bool isOrderPreserved() const { return _preserveOrder; }

    /// \brief Get number of view in parallel
    /// \return Number of view in parallel
    uint32_t getNumberOfViewParallel() const { return _numberOfViewParallel; }

    /// \brief Get number of tiles to cache
    /// \return Number of tiles to cache
    uint32_t getNumberOfTilesToCache() const { return _numberOfTilesToCache; }

    /// \brief Get number of tiles loader
    /// \return number of tiles loader
    uint32_t getNumberOfTileLoader() const { return _numberOfTileLoader; }

    /// \brief Get traversal type
    /// \return Traversal type
    TraversalType getTraversalType() const { return _traversalType; }

    /// \brief Get filling type
    /// \return
    FillingType getFillingType() const { return _fillingType; }

    /// \brief Get number of release count per pyramid level
    /// \param pyramidLvl Pyramid level
    /// \return Number of release count associate with the pyramid level
    uint32_t getNbReleasePyramid(const size_t pyramidLvl) const {
      assert(pyramidLvl < _nbReleasePyramid.size());
      return _nbReleasePyramid[pyramidLvl];
    }

    /// \brief Get all the release count
    /// \return Vector with all the release count
    const std::vector<uint32_t> &getNbReleasePyramid() const {
      return _nbReleasePyramid;
    }

    /// \brief Set if the order is preserved
    /// \param preserveOrder true if the order has to be preserved, else false
    void setPreserveOrder(bool preserveOrder) { _preserveOrder = preserveOrder; }

    /// \brief Set if has finish requesting views
    /// \param finishRequestingViews True if finish requesting views, False else
    void setFinishRequestingViews(bool finishRequestingViews) {
      _finishRequestingViews = finishRequestingViews;
    }

    /// \brief Set number of views could be loaded in parallel
    /// \param numberOfViewParallel Number of views could be loaded in parallel
    void setNumberOfViewParallel(uint32_t numberOfViewParallel) {
      _numberOfViewParallel = numberOfViewParallel;
    }

    /// \brief Set number of tiles to cache
    /// \param numberOfTilesToCache Number of tiles to cache
    void setNumberOfTilesToCache(uint32_t numberOfTilesToCache) {
      _numberOfTilesToCache = numberOfTilesToCache;
    }

    /// \brief Set number of tile loader
    /// \param numberOfTileLoader Number of tile loader
    void setNumberOfTileLoader(uint32_t numberOfTileLoader) {
      _numberOfTileLoader = numberOfTileLoader;
    }

    /// \brief Set traversal pattern to traverse the image
    /// \param traversalType Traversal pattern to traverse the image
    void setTraversalType(TraversalType traversalType) {
      _traversalType = traversalType;
    }

    /// \brief Set filling type for the ghost region
    /// \param fillingType Filling type for the ghost region
    void setFillingType(FillingType fillingType) { _fillingType = fillingType; }

    /// \brief Set the release count for a specific pyramid level
    /// \param pyramidLvl Pyramid level
    /// \param nbRelease  Release count to set
    void setNbReleasePyramid(const size_t pyramidLvl, const uint32_t nbRelease) {
      assert(pyramidLvl < _nbReleasePyramid.size());
      _nbReleasePyramid[pyramidLvl] = nbRelease;
    }

   private:
    bool
        _finishRequestingViews = false,         ///< Is Finish sent
        _preserveOrder = false;                 ///< True if output order is
                                                ///< the same as the requested
                                                ///< order

    uint32_t
        _numberOfViewParallel = 1,              ///< Number of views available
                                                ///< in parallel
        _numberOfTilesToCache = 0,              ///< Number of tiles to cache
        _numberOfTileLoader = 1;                ///< Number of tiles loader

    TraversalType
        _traversalType = TraversalType::SNAKE;  ///< Traversal type

    FillingType
        _fillingType = FillingType::FILL;       ///< Filling for ghost region

    std::vector<uint32_t>
        _nbReleasePyramid;                      ///< Number of time we need to
                                                ///< release a view per levels
  };
 public:
  /// \brief FastImage constructor
  /// \param tileLoader Image TileLoader
  /// \param radius Number of pixel around the central tile
  /// \details Construct the Fast Image object and set up the defaults values,
  /// plus the given ATileLoader.
  FastImage(ATileLoader<UserType> *tileLoader, const uint32_t &radius)
        : _radius(radius),
          _numberTilesFeatureComputed(0),
          _numberTilesFeatureTotal(0),
          _runtime(nullptr),
          _tileLoader(tileLoader) {
    assert(tileLoader != nullptr);
    // Get the value from the loaded images
    _fastImageOptions = new Options(_tileLoader->getNbPyramidLevels());
  }

  /// \brief FastImage constructor
  /// \param tileLoader std::unique_ptr to the image TileLoader
  /// \param radius Number of pixel around the central tile
  /// \details Construct the Fast Image object and set up the defaults values,
  /// plus the given ATileLoader.
  FastImage(std::unique_ptr<ATileLoader<UserType>> tileLoader,
            const uint32_t &radius)
      : _radius(radius), _numberTilesFeatureComputed(0),
        _numberTilesFeatureTotal(0), _runtime(nullptr),
        _tileLoader(std::move(tileLoader).release()) {
    assert(_tileLoader != nullptr);
    // Get the value from the loaded images
    _fastImageOptions = new Options(_tileLoader->getNbPyramidLevels());
  }

  /// \brief FastImage destructor.
  /// \details Destroy the object. Wait for the graph to complete, delete the
  /// options, the different levels of cache, and the graph runtime.
  ~FastImage() {
    waitForGraphComplete();
    delete _fastImageOptions;
    // Delete the cache
    for (auto &cache: _allCache) { delete cache; }

    // Delete the graph
    delete _runtime;
  }

  /// \brief Set up and run the graph.
  /// \details Configure the FastImage object and launch the runtime. The
  /// options are parsed and applied. The different graph's tasks (ViewLoader,
  /// ATileLoader, ViewCounter) are created. The memory for the graph is
  /// allocated.
  void configureAndRun() {
    if (!_hasBeenConfigured) {
      configure();
      // Launch the graph
      _runtime = new htgs::TaskGraphRuntime(_taskGraph);
      _runtime->executeRuntime();
    }
  }

  /// \brief Set up and create a TGTask from the instance of FI given.
  /// \details Configure the FastImage object and wrap it into a TGTask.
  /// The options are parsed and applied. The different graph's tasks
  /// (ViewLoader, ATileLoader, ViewCounter) are created. The memory for the
  /// graph is allocated.
  /// The TGTask has as input a ViewRequestData<UserType> and as output
  /// htgs::MemoryData<View<UserType>>
  /// We recommend two methods when working with the resuling TGTask::
  /// 1: The TGTask can accept a ViewRequestData and produce the asked
  /// views.
  /// 2: The FI API can be called directly on the FI instance, which will
  /// directly produce data for the graph that is within the TGTask. This works
  /// because the instance of the FI graph is wrapped into the TGTask.
  /// \warning
  /// 1: If the TGTask is copied (internally in an execution
  /// pipeline) or by hand, then direct calls to the FI API will result in data
  /// not being sent correctly.
  /// 2: The FI pointer need to be deleted
  /// \param name TGTask Name
  /// \return TGTask wraping a FI instance
  htgs::TGTask<ViewRequestData<UserType>, htgs::MemoryData<View<UserType>>>*
      configureAndMoveToTaskGraphTask(std::string name = "FastImageTask"){
    if(_hasBeenConfigured){
      std::stringstream message;
      message
      << "FastImage has already seen it configuration applied, please call "
         "directly configureAndMoveToTaskGraphTask() instead of "
         "configureAndRun().";
      std::string m = message.str();
      throw (FastImageException(m));
    }

    configure();

    return _taskGraph->createTaskGraphTask(name, true);
  }

  /// Get the view radius
  /// \return The view radius
  uint32_t getRadius() const { return _radius; }

  /// \brief Get Image width in px
  /// \param level Pyramid level
  /// \return Image width in px
  uint32_t getImageWidth(uint32_t level = 0) const {
    assert(level <= this->_tileLoader->getNbPyramidLevels());
    return this->_tileLoader->getImageWidth(level);
  }

  /// \brief Get Image height in px
  /// \param level Pyramid level
  /// \return Image height in px
  uint32_t getImageHeight(uint32_t level = 0) const {
    assert(level <= this->_tileLoader->getNbPyramidLevels());
    return this->_tileLoader->getImageHeight(level);
  }

  /// \brief Get Tile width in px
  /// \param level Pyramid level
  /// \return Tile width in px
  uint32_t getTileWidth(uint32_t level = 0) const {
    assert(level <= this->_tileLoader->getNbPyramidLevels());
    return this->_tileLoader->getTileWidth(level);
  }

  /// \brief Get Tile height in px
  /// \param level Pyramid level
  /// \return Tile height in px
  uint32_t getTileHeight(uint32_t level = 0) const {
    assert(level <= this->_tileLoader->getNbPyramidLevels());
    return this->_tileLoader->getTileHeight(level);
  }

  /// \brief Get view height
  /// \param level Pyramid level
  /// \return View height
  uint32_t getViewHeight(uint32_t level = 0) const {
    return getTileHeight(level) + 2 * getRadius();
  }

  /// \brief Get view width
  /// \param level Pyramid level
  /// \return View width
  uint32_t getViewWidth(uint32_t level = 0) const {
    return getTileWidth(level) + 2 * getRadius();
  }

  /// \brief Get number of tiles in a column
  /// \param level Pyramid level
  /// \return Number of tiles in a column
  uint32_t getNumberTilesHeight(uint32_t level = 0) const {
    return (uint32_t) ceil(
        (double) getImageHeight(level) / getTileHeight(level));
  }

  /// \brief Get number of tiles in a row
  /// \param level Pyramid level
  /// \return Number of tiles in a row
  uint32_t getNumberTilesWidth(uint32_t level = 0) const {
    return (uint32_t) ceil((double) getImageWidth(level) / getTileWidth(level));
  }

  /// \brief Get Number of pyramid levels
  /// \return Number of pyramid levels
  uint32_t getNbPyramidLevels() const {
    return this->_tileLoader->getNbPyramidLevels();
  }

  /// \brief Get the Fast Image options
  /// \return Fast Image options
  Options *getFastImageOptions() const { return _fastImageOptions; }

  /// \brief Get an available (fully loaded) view
  /// \return Available view
  htgs::m_data_t<View<UserType>> getAvailableViewBlocking() {
    auto view = this->getTaskGraph()->consumeData();
    this->incrementTileFeatureComputed();
    return view;
  }

  /// \brief Get the Hit and miss cache access
  /// \param level Pyramid level
  /// \return pair<hit, miss>
  std::pair<uint32_t, uint32_t>
  getHitMissCache(uint32_t level = 0) {
    return _allCache[level]->getHitMissCache();
  }

  /// \brief Get the image size in Bytes
  /// \param level Pyramid level
  /// \return The image size in Bytes
  double getImageSizeMBytes(uint32_t level = 0) {
    assert(level <= this->_tileLoader->getNbPyramidLevels());
    return ((double) this->getImageWidth(level)
        * (double) this->getImageHeight(level)
        * ((double) this->_tileLoader->getBitsPerSample() / 8))
        / (1024. * 1024.);
  }

  /// \brief Get the number of tiles in a feature already computed
  /// \return The number of tiles in a feature
  uint32_t getNumberTilesFeatureComputed() const {
    return _numberTilesFeatureComputed;
  }

  /// \brief Get the number of total tiles in a feature
  /// \return The number of tiles in a feature
  uint32_t getNumberTilesFeatureTotal() const {
    return _numberTilesFeatureTotal;
  }

  /// \brief Get the information is a feature has been totally computed
  /// \return Is a feature has been totally computed
  bool isFeatureDone() {
    return this->getNumberTilesFeatureComputed()
        >= this->getNumberTilesFeatureTotal();
  }

  /// \brief Get the information if the graph is still processing tiles
  /// \return True If the graph is still processing tile, False else
  bool isGraphProcessingTiles() { return !_taskGraph->isOutputTerminated(); }

  /// \brief Set the number of tiles in a feature already computed
  /// \param numberTilesFeatureComputed The number of tiles computed
  void setNumberTilesFeatureComputed(
      uint32_t numberTilesFeatureComputed) {
    _numberTilesFeatureComputed = numberTilesFeatureComputed;
  }

  /// \brief Set the number of total tiles in a feature
  /// \param numberTilesFeatureTotal The number of total tiles in a feature
  void setNumberTilesFeatureTotal(
      uint32_t numberTilesFeatureTotal) {
    _numberTilesFeatureTotal = numberTilesFeatureTotal;
  }

  /// \brief Indicate to the system no more request will be done
  void finishedRequestingTiles() {
    if (!_fastImageOptions->isFinishedRequestingViews()) {
      _taskGraph->finishedProducingData();
      _fastImageOptions->setFinishRequestingViews(true);
    }
  }

  /// \brief Wait to the graph to complete
  void waitForGraphComplete() {
    if (!isFinishedRequestingViews()) { finishedRequestingTiles(); }
    if(_runtime != nullptr){
      _runtime->waitForRuntime();
    }
  }

  /// \brief Request a tile
  /// \details Request a tile at indexes (rowIndex, colIndex) to the ViewLoader
  /// at a specific pyramid level.
  /// \param rowIndex Row tile index
  /// \param colIndex Column tile index
  /// \param finishRequestingTiles True if the end user has finished to request
  /// views, else False.
  /// \param level Pyramid level
  void requestTile(uint32_t rowIndex,
                   uint32_t colIndex,
                   uint32_t level,
                   bool finishRequestingTiles) {
    assert(rowIndex < this->getNumberTilesHeight());
    assert(colIndex < this->getNumberTilesWidth());
    assert(_hasBeenConfigured);

    if (this->isFinishedRequestingViews()) {
      return;
    }

    std::queue<std::pair<uint32_t, uint32_t>> fifo;
    fifo.push(std::make_pair(rowIndex, colIndex));
    _viewCounter->addTraversal(fifo);
    sendRequest(rowIndex, colIndex, level);
    if (finishRequestingTiles) {
      this->finishedRequestingTiles();
    }
  }

  /// \brief Request a specific feature into a features collection
  /// \details Requests all the views which compose a specific fc::Feature at a
  /// specific pyramid level. All these requests are send to the ViewLoader.
  /// \param feature Features collection's feature
  /// \param level Pyramid level
  void requestFeature(const fc::Feature &feature, uint32_t level = 0) {
    assert(_hasBeenConfigured);
    const fc::BoundingBox &bB = feature.getBoundingBox();

    uint32_t
        indexRowMin = bB.getUpperLeftRow() / (this->getTileHeight()),
        indexRowMax = 0,
        indexColMin = bB.getUpperLeftCol() / this->getTileWidth(),
        indexColMax = 0;

    // Handle border case
    if (bB.getBottomRightCol() == this->getImageWidth())
      indexColMax = this->getNumberTilesWidth();
    else
      indexColMax = (bB.getBottomRightCol() / this->getTileWidth()) + 1;
    if (bB.getBottomRightRow() == this->getImageHeight())
      indexRowMax = this->getNumberTilesHeight();
    else
      indexRowMax = (bB.getBottomRightRow() / this->getTileHeight()) + 1;

    this->setNumberTilesFeatureComputed(0);
    this->setNumberTilesFeatureTotal(
        (indexRowMax - indexRowMin) * (indexColMax - indexColMin));

    std::queue<std::pair<uint32_t, uint32_t>> fifo;

    for (auto indexRow = indexRowMin; indexRow < indexRowMax; ++indexRow) {
      for (auto indexCol = indexColMin; indexCol < indexColMax; ++indexCol) {
        fifo.push(std::make_pair(indexRow, indexCol));
      }
    }

    _viewCounter->addTraversal(fifo);

    for (auto indexRow = indexRowMin; indexRow < indexRowMax; ++indexRow) {
      for (auto indexCol = indexColMin; indexCol < indexColMax; ++indexCol) {
        sendRequest(indexRow, indexCol, level);
      }
    }
  }

  /// \brief Request all tiles following a traversal
  /// \param finishRequestingTiles True if the end user has finished to request
  /// views, else False.
  /// \param level Pyramid level
  void requestAllTiles(bool finishRequestingTiles, uint32_t level = 0) {
    assert(_hasBeenConfigured);
    if (this->isFinishedRequestingViews())
      return;

    Traversal traversal = Traversal(this->_fastImageOptions->getTraversalType(),
                                    getNumberTilesHeight(level),
                                    getNumberTilesWidth(level));

    _viewCounter->addTraversal(traversal.getQueue());

    for (auto step : traversal.getTraversal()) {
      sendRequest(step.first, step.second, level);
    }

    if (finishRequestingTiles) {
      this->finishedRequestingTiles();
    }
  }

  /// \brief Increment the number of tiles for a region already computed
  void incrementTileFeatureComputed() {
    std::mutex mtx;
    std::lock_guard<std::mutex> lck(mtx);
    this->_numberTilesFeatureComputed += 1;
  }

  /// \brief Create a dot file at the path filename
  /// \param filename path for the dot file
  /// \param flags to print the graph
  void writeGraphDotFile(const std::string &filename, int flags = 0) {
    _taskGraph->writeDotToFile(filename, flags);
  }

 private:
  /// \brief Get the HTGS task graph
  /// \return The task graph
  htgs::TaskGraphConf<ViewRequestData<UserType>,
                      htgs::MemoryData<View<UserType>>> *getTaskGraph() const {
    return _taskGraph;
  }

  /// \brief Set up the graph.
  /// \details Configure the FastImage object. The options are parsed and
  /// applied. The different graph's tasks (ViewLoader, ATileLoader,
  /// ViewCounter) are created. The memory for the graph is allocated.
  void configure(){
    if (!_hasBeenConfigured) {
      assert(_fastImageOptions->getNumberOfViewParallel() > 0);
      _hasBeenConfigured = true;
      if (_fastImageOptions->getNumberOfTileLoader() == 0)
        _fastImageOptions->setNumberOfTileLoader(1);

      ViewLoader<UserType> *viewLoader = nullptr;
      _viewCounter = nullptr;

      std::vector<size_t> numViewsParallel;
      std::vector<std::shared_ptr<htgs::IMemoryAllocator<View<UserType>>>>
          viewAllocators;

      for (uint32_t level = 0; level < _tileLoader->getNbPyramidLevels(); level++) {
        // Create the cache
        auto cache =
            new FigCache<UserType>(
                this->_fastImageOptions->getNumberOfTilesToCache()
            );
        // Init the cache
        cache->initCache(this->getNumberTilesHeight(level),
                         this->getNumberTilesWidth(level),
                         this->getTileHeight(level),
                         this->getTileWidth(level));
        _allCache.push_back(cache);

        size_t
            numViewParallelTemp = _fastImageOptions->getNumberOfViewParallel();

        // Set the number of view parallel
        if (getNumberTilesWidth(level) * getNumberTilesHeight(level)
            < _fastImageOptions->getNumberOfViewParallel()) {
          numViewParallelTemp =
              getNumberTilesWidth(level) * getNumberTilesHeight(level);
        }

        numViewsParallel.push_back(numViewParallelTemp);
        auto viewAllocator =
            std::shared_ptr<ViewAllocator<UserType>>(
                new ViewAllocator<UserType>(
                    getViewHeight(level),
                    getViewWidth(level)
                )
            );
        viewAllocators.push_back(viewAllocator);
      }

      auto memManager =
          new VariableMemoryManager<View<UserType>>("viewMem",
                                                    numViewsParallel,
                                                    viewAllocators,
                                                    htgs::MMType::Static);

      // Create the Fast Image graph
      _taskGraph = new htgs::TaskGraphConf<ViewRequestData<UserType>,
                                           htgs::MemoryData<View<UserType>>>();

      // Set the cache
      _tileLoader->setCache(_allCache);

      // Init the graph's parts
      viewLoader =
          new ViewLoader<UserType>(
              this->_fastImageOptions->getNbReleasePyramid()
          );
      _viewCounter =
          new ViewCounter<UserType>(_fastImageOptions->getFillingType(),
                                    _fastImageOptions->isOrderPreserved());

      if (this->getNbPyramidLevels() == 1) {
        // Set graph parts
        _taskGraph->setGraphConsumerTask(viewLoader);
        _taskGraph->addEdge(viewLoader, _tileLoader);
        _taskGraph->addEdge(_tileLoader, _viewCounter);
        _taskGraph->addGraphProducerTask(_viewCounter);

        _taskGraph->addCustomMemoryManagerEdge(viewLoader, memManager);
      } else {
        auto pyramidGraph =
            new htgs::TaskGraphConf<ViewRequestData<UserType>,
                                    htgs::MemoryData<View<UserType>>>();
        pyramidGraph->setGraphConsumerTask(viewLoader);
        pyramidGraph->addEdge(viewLoader, _tileLoader);
        pyramidGraph->addEdge(_tileLoader, _viewCounter);
        pyramidGraph->addGraphProducerTask(_viewCounter);
        pyramidGraph->addCustomMemoryManagerEdge(viewLoader, memManager);
        auto
            execPipeline =
            new htgs::ExecutionPipeline<ViewRequestData<UserType>,
                                        htgs::MemoryData<View<UserType>>>(
                this->getNbPyramidLevels(), pyramidGraph);
        auto distributeRule = new DistributePyramidRule<UserType>();
        execPipeline->addInputRule(distributeRule);
        _taskGraph->setGraphConsumerTask(execPipeline);
        _taskGraph->addGraphProducerTask(execPipeline);
      }
    }
  }

  /// \brief Test if the system has finished sending tiles
  /// \return true if the system has finished sending tiles, false either
  bool isFinishedRequestingViews() const {
    return _fastImageOptions->isFinishedRequestingViews();
  }

  /// \brief Send a View request to the ViewLoader about the tile at
  /// coordinate (indexTileRow, indexTileCol)
  /// \param indexTileRow Row's index of the view's center tile asked
  /// \param indexTileCol Col's index of the view's center tile asked
  /// \param level Pyramid level
  void sendRequest(uint32_t indexTileRow,
                   uint32_t indexTileCol,
                   uint32_t level = 0) {
    assert(level <= this->_tileLoader->getNbPyramidLevels());
    _taskGraph->produceData(
        new ViewRequestData<UserType>(
            indexTileRow, indexTileCol,
            getNumberTilesHeight(level), getNumberTilesWidth(level),
            this->getRadius(), getTileHeight(level), getTileWidth(level),
            getImageHeight(level), getImageWidth(level), level)
    );
  }

  uint32_t
      _radius,                        ///< View radius, i.e. number of pixels in
                                      ///< each direction around the center tile
      _numberTilesFeatureComputed,    ///< Number of tiles treated for a feature
      _numberTilesFeatureTotal;       ///< Number of tiles in totals for a
                                      ///< feature

  htgs::TaskGraphConf<ViewRequestData<UserType>,
                      htgs::MemoryData<View<UserType>>> *
      _taskGraph;                     ///< HTGS Graph to load the views and the
                                      ///< tiles

  htgs::TaskGraphRuntime *
      _runtime;                       ///< HTGS Graph runtime

  ATileLoader<UserType> *
      _tileLoader;                    ///< Tile Loader

  ViewCounter<UserType> *
      _viewCounter;                   ///< View Counter

  std::vector<FigCache<UserType> *>
      _allCache;                      ///< Tile Caches given to each tile loader

  Options *
      _fastImageOptions = nullptr;    ///< Fast Image Options

  bool
      _hasBeenConfigured = false;     ///< Private flag to be sure the FI is
                                      ///< configure
};
}

#endif //FASTIMAGE_H
