#include "stdafx.h"
#include <MFCUtil/ProgressWindow.h>
#include <MFCUtil/resource.h>
#include <CPUInfo.h>
#include <SynchronizedQueue.h>
#include "EndGameTablebase.h"

#ifndef TABLEBASE_BUILDER

class DecompressSingleJob {
private:
  const EndGameTablebase *m_tb;
  UINT64                  m_compressedSize;
public:
  DecompressSingleJob() : m_tb(NULL), m_compressedSize(0) {
  }
  DecompressSingleJob(EndGameTablebase *tb) 
    : m_tb(tb)
    , m_compressedSize(tb->getFileSize(COMPRESSEDTABLEBASE))
  {
  }
  inline const EndGameTablebase *getTablebase() const {
    return m_tb;
  }
  inline UINT64 getFileSize() const {
    return m_compressedSize;
  }
};

class DecompressJobQueue : public SynchronizedQueue<DecompressSingleJob> {
private:
  UINT64    m_sumFileSize;
  Semaphore m_gate;
public:
  DecompressJobQueue(const EndGameTablebaseList &list) : m_sumFileSize(0) {
    for (size_t i = 0; i < list.size(); i++) {
      DecompressSingleJob job(list[i]);
      put(job);
      m_sumFileSize += job.getFileSize();
    }
  }
  UINT64 getSumFileSize() const {
    return m_sumFileSize;
  }
};

class Decompressor : public InterruptableRunnable, public ByteCounter {
private:
  DecompressJobQueue        &m_jobQueue;
  mutable Semaphore          m_gate;
  DecompressSingleJob        m_currentJob;
  String                     m_currentMsg;
  UINT64                     m_byteCounterStart, m_currentFileSize;
  inline void setCurrentFileSize() {
    m_byteCounterStart = getCount();
    m_currentFileSize  = m_currentJob.getFileSize();
  }
  inline void setCurrentMessage(const String &msg) {
    m_gate.wait();
    m_currentMsg = msg;
    m_gate.signal();
  }
public:
  Decompressor(DecompressJobQueue &jobQueue);
  ~Decompressor();
  inline USHORT getSubProgress() {
    return (USHORT)PERCENT(getCount() - m_byteCounterStart, m_currentFileSize);
  }
  String getCurrentMessage() const {
    m_gate.wait();
    const String result = m_currentMsg;
    m_gate.signal();
    return result;
  }
  void incrCount(UINT64 n) {
    checkInterruptAndSuspendFlags();
    ByteCounter::incrCount(n);
  }
  UINT getMaxChunkSize() const {
    return 100000;
  }
  UINT run();
};

Decompressor::Decompressor(DecompressJobQueue &jobQueue) : m_jobQueue(jobQueue) {
  setSelectedLanguageForThread();
  setCurrentFileSize();
}

Decompressor::~Decompressor() {
  setInterrupted();
  while(!isTerminated()) {
    Sleep(200);
  }
}

UINT Decompressor::run() {
  try {
    for (;;) {
      checkInterruptAndSuspendFlags();
      try {
        m_currentJob = m_jobQueue.get(500);
      } catch (...) {
        if(!m_jobQueue.isEmpty()) continue;
        break;
      }
      checkInterruptAndSuspendFlags();
      setCurrentMessage(m_currentJob.getTablebase()->getName());
      setCurrentFileSize();
      m_currentJob.getTablebase()->decompress(this);
      verbose(_T("\n"));
    }
    setCurrentMessage(_T("Done"));
  } catch (...) {
    // do nothing
  }
  setTerminated();
  return 0;
}

class DecompressJob : public InteractiveRunnable {
private:
  DecompressJobQueue              m_jobQueue;
  UINT64                          m_startSumFileSize;
  CompactArray<Decompressor*>     m_decompArray;
  String                          m_title;
  void   clearDecompArray();
  void   suspendAll();
  void   resumeAll();
  void   interruptAll();
  void   waitUntilAllTerminated();
  bool   anyDecompressorsActive() const;
  UINT64 sumDecompressorsBytesDone() const;
public:
  DecompressJob(const EndGameTablebaseList &list);
  ~DecompressJob();
  double getMaxProgress() const {         // Only called if getSupportedFeatures() contains IR_PROGRESSBAR, IR_SHOWTIMEESTIMATE or IR_SUBPROGRESSBAR
    return (double)m_startSumFileSize;
  }
  double getProgress() const {            // do. Should return a value in the range [0..getMaxProgress()]
    return (double)sumDecompressorsBytesDone();
  };
  USHORT getSubProgressPercent(UINT index) {  // Only called if getSupportedFeatures() contains IR_SUBPROGRESSBAR
    return m_decompArray[index]->getSubProgress();
  }
  USHORT getJobCount() const {
    return (USHORT)m_decompArray.size();
  }
  String getProgressMessage(UINT index) {
    return m_decompArray[index]->getCurrentMessage();
  }
  String getTitle() {
    return m_title;
  }
  int getSupportedFeatures() {
    return IR_PROGRESSBAR     | IR_SUBPROGRESSBAR 
         | IR_INTERRUPTABLE   | IR_SUSPENDABLE 
         | IR_SHOWPROGRESSMSG | IR_SHOWTIMEESTIMATE;
  }
  UINT run();
};

DecompressJob::DecompressJob(const EndGameTablebaseList &list) : m_jobQueue(list) {
  setSelectedLanguageForThread();
  m_startSumFileSize = m_jobQueue.getSumFileSize();
#ifdef _DEBUG
  const int cpuCount = 1;
#else
  const int cpuCount = getProcessorCount();
#endif // _DEBUG
  for (int i = 0; i < cpuCount; i++) {
    m_decompArray.add(new Decompressor(m_jobQueue));
  }

  m_title = format(_T("%s - %s:%s")
                 ,loadString(IDS_DECOMPRESSALLTITLE).cstr()
                 ,loadString(IDS_METRIC).cstr()
                 ,EndGameKeyDefinition::getMetricName()
                  );
}

DecompressJob::~DecompressJob() {
  clearDecompArray();
}

bool DecompressJob::anyDecompressorsActive() const {
  for (size_t i = 0; i < m_decompArray.size(); i++) {
    if(!m_decompArray[i]->isTerminated()) {
      return true;
    }
  }
  return false;
}

UINT64 DecompressJob::sumDecompressorsBytesDone() const {
  UINT64 sum = 0;
  for (size_t i = 0; i < m_decompArray.size(); i++) {
    sum += m_decompArray[i]->getCount();
  }
  return sum;
}

void DecompressJob::clearDecompArray() {
  interruptAll();
  for (size_t i = 0; i < m_decompArray.size(); i++) {
    delete m_decompArray[i];
  }
  m_decompArray.clear();
}

void DecompressJob::suspendAll() {
  for (size_t i = 0; i < m_decompArray.size(); i++) {
    m_decompArray[i]->setSuspended();
  }
}

void DecompressJob::resumeAll() {
  for (size_t i = 0; i < m_decompArray.size(); i++) {
    m_decompArray[i]->resume();
  }
}

void DecompressJob::interruptAll() {
  for (size_t i = 0; i < m_decompArray.size(); i++) {
    m_decompArray[i]->setInterrupted();
  }
  waitUntilAllTerminated();
}

void DecompressJob::waitUntilAllTerminated() {
  for(;;) {
    if(!anyDecompressorsActive()) {
      return;
    }
    Sleep(1000);
  }
}

UINT DecompressJob::run() {
  for (size_t i = 0; i < m_decompArray.size(); i++) {
    ThreadPool::executeNoWait(*m_decompArray[i]);
  }
  while(!m_jobQueue.isEmpty() || anyDecompressorsActive()) {
    if(isSuspendOrCancelButtonPressed()) {
      if(isInterrupted()) {
        interruptAll();
        break;
      } else if(isSuspended()) {
        suspendAll();
        suspend();
        resumeAll();
      }
    }
    Sleep(400);
  }
  return 0;
}

static EndGameTablebaseList getNotDecompressedEndGameTablebases() {
  EndGameTablebaseList result = EndGameTablebase::getExistingEndGameTablebases();
  for(size_t i = result.size(); i--;) {
    if(!result[i]->needDecompress()) {
      result.remove(i);
    }
  }
  return result;
}

void EndGameTablebase::decompressAll() { // static
  EndGameTablebaseList jobList = getNotDecompressedEndGameTablebases();
  if(jobList.size() == 0) {
    AfxMessageBox(IDS_ALLTABLEBASESDECOMPRESSED, MB_ICONINFORMATION|MB_OK);
  } else {
    DecompressJob job(jobList);
    ProgressWindow(NULL, job, 0, 2000);
    if (job.isInterrupted()) {
      AfxMessageBox(_IDS_INTERRUPTED_BY_USER, MB_ICONEXCLAMATION|MB_OK);
    } else {
      AfxMessageBox(IDS_ALLTABLEBASESDECOMPRESSED, MB_ICONINFORMATION|MB_OK);
    }
  }
}

#endif
