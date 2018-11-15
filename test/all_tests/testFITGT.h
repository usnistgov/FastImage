//
// Created by anb22 on 11/15/18.
//

#ifndef FASTIMAGE_TESTFITGT_H
#define FASTIMAGE_TESTFITGT_H

#include <FastImage/api/FastImage.h>
#include <FastImage/TileLoaders/GrayscaleTiffTileLoader.h>

class uint64Data : public IData {
  uint64_t _myint{};
 public:
  uint64Data() : _myint(0) {}
  uint64Data(uint64_t myint) : _myint(myint) {}
  uint64_t myint() const { return _myint; }
  void setMyint(uint64_t myint) { _myint = myint; }
  uint64Data &operator+=(const uint64Data & oth){
    this->_myint += oth.myint();
    return *this;
  }
  uint64Data &operator+=(const uint64_t & oth){
    this->_myint += oth;
    return *this;
  }
};

class AddTask : public htgs::ITask<
    htgs::MemoryData<fi::View<uint8_t>>,
    uint64Data
    >{
 public:
  AddTask() {}

  void executeTask(std::shared_ptr<MemoryData<fi::View<uint8_t>>> data) {
    if(data != nullptr){
      auto *result = new uint64Data();
      auto view = data->get();
      for(auto row = 0; row < view->getTileHeight(); ++row){
        for(auto col = 0; col < view->getTileWidth(); ++col){
          result ->operator+= (view->getPixel(row, col));
        }
      }
      std::cout << view->getRow() << ", " << view->getCol() << ":" << result->myint() << std::endl;
      data->releaseMemory();

      this->addResult(result);
    }
  }

 public:
  ITask<MemoryData<fi::View<uint8_t>>, uint64Data> *copy() override {
    return nullptr;
  }
};

void testFITGTask(){
  auto tileLoader = new fi::GrayscaleTiffTileLoader<uint8_t>("mosaic.tif");
  auto fi = new fi::FastImage<uint8_t>(tileLoader, 0);
  fi->getFastImageOptions()->setPreserveOrder(true);
  auto tgtFI = fi->configureAndMoveToTaskGraphTask();
  AddTask *addTask = new AddTask();

  auto mainGraph = new htgs::TaskGraphConf<htgs::VoidData, uint64Data>();
  mainGraph->addEdge(tgtFI, addTask);
  mainGraph->addGraphProducerTask(addTask);

  auto runtime = new htgs::TaskGraphRuntime(mainGraph);

  runtime->executeRuntime();

  fi->requestAllTiles(true);

  while (!mainGraph->isOutputTerminated()) {
    auto result = mainGraph->consumeData();
    if(result != nullptr){
    }
  }

  runtime->waitForRuntime();
  delete runtime;
}



#endif //FASTIMAGE_TESTFITGT_H
