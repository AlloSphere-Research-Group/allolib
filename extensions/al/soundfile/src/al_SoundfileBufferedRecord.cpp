#include <cassert>
#include <iostream>

#include "al/soundfile/al_SoundfileBufferedRecord.hpp"

using namespace al;

SoundFileBufferedRecord::SoundFileBufferedRecord() :
  mRunning(false),
  mCurPos(0)
{
}

SoundFileBufferedRecord::~SoundFileBufferedRecord()
{
  close();
}

bool SoundFileBufferedRecord::open(std::string fullPath, double frameRate, uint32_t numChannels, uint32_t bufferFrames, Format format,
                                   EncodingType encoding)
{
  close();
  mSf.frameRate(frameRate);
  mSf.channels(int(numChannels));
  mSf.format(format);
  mSf.encoding(encoding);
  mBufferFrames = bufferFrames;

  mRingBuffer = new SingleRWRingBuffer(mBufferFrames * numChannels * sizeof(float));
  mFileBuffer = std::make_unique<float []>(mBufferFrames * numChannels);
  std::condition_variable cond;
  std::mutex condMutex;
  {
    std::unique_lock<std::mutex> lk(condMutex);
    mReaderThread = new std::thread(writeFunction, this, &cond, &condMutex);
    cond.wait(lk); // Wait for thread to have started
  }

  mSf.openWrite(fullPath);
  if (!mSf.opened()) {
    std::cerr << "ERROR opening sound file in " << __FUNCTION__ << std::endl;
    cleanup();
    return false;
  }
  return true;
}

void SoundFileBufferedRecord::cleanup()
{
  if (mReaderThread &&  mRingBuffer) {
    mRunning = false;
    mCondVar.notify_one();
    mReaderThread->join();
    delete mReaderThread;
    delete mRingBuffer;
    mReaderThread = nullptr;
    mRingBuffer = nullptr;
  }
}

bool SoundFileBufferedRecord::close()
{

  if (mSf.opened()) {
    mSf.close();
  }
  return true;
}

void SoundFileBufferedRecord::write(std::vector<float *> buffers, size_t numFrames)
{
  size_t channel = 0;
  // FIXME we should do the interleaving in the writing thread
  assert(buffers.size() == mSf.channels());
  for(auto *buf: buffers) {
    float *destBuf = mFileBuffer.get() + channel++;
    for (size_t i = 0; i < numFrames; i++) {
      *destBuf = *buf++;
      destBuf += mSf.channels();
    }
  }
  size_t bytesWritten = mRingBuffer->write((char *) mFileBuffer.get(), numFrames * mSf.channels() * sizeof(float));
  if (bytesWritten != numFrames * mSf.channels() * sizeof(float)) {
    std::cerr << "Recording buffer overrun. Increase buffer size" << std::endl;
  }
  mCondVar.notify_one();
  return;
}

bool SoundFileBufferedRecord::opened() const
{
  return mSf.opened();
}

void SoundFileBufferedRecord::writeFunction(SoundFileBufferedRecord  *obj, std::condition_variable *cond, std::mutex *condMutex)
{
  obj->mRunning = true;
  float *writeBuffer = new float[size_t(obj->mBufferFrames * obj->mSf.channels())];
  condMutex->lock();
  cond->notify_all(); // Signal thread is processing;
  condMutex->unlock();
  while (obj->mRunning) {
    std::unique_lock<std::mutex> lk(obj->mLock);
    obj->mCondVar.wait(lk);
    int bytesToWrite = obj->mRingBuffer->read((char *) writeBuffer, obj->mSf.frameRate() * obj->mSf.channels() * sizeof(float));
    int framesToWrite = bytesToWrite/(obj->mSf.channels() * sizeof(float));
    int framesWritten = obj->mSf.write<float>(obj->mFileBuffer.get(), framesToWrite);
//    std::cout << "Wrote " << framesWritten << std::endl;
    std::atomic_fetch_add(&(obj->mCurPos), framesWritten);

    if (obj->mMaxWriteTime > 0 && obj->mCurPos >= obj->mMaxWriteTime * obj->mSf.frameRate()) {
      std::cout << "SoundFileBufferedRecord max time exceeded. Closing sound file" << std::endl;
      obj->mRunning = false;
      obj->mSf.close();
    }
    //		if (written != framesWritten * sizeof(float) * obj->channels()) {
    //			// TODO handle overrun
    //		}
//    if (obj->mReadCallback) {
//      obj->mReadCallback(obj->mFileBuffer, obj->mSf.channels(), framesWritten, obj->mCallbackData);
//    }
    lk.unlock();
  }
  delete[] writeBuffer;
}

//void SoundFileBufferedRecord::setWriteCallback(SoundFileBufferedRecord::CallbackFunc func, void *userData)
//{
//  mReadCallback = func;
//  mCallbackData = userData;
//}

int SoundFileBufferedRecord::currentPosition()
{
  return mCurPos.load();
}
