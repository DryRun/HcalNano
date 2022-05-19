#!/bin/bash
echo "Starting LZMA 9"
time cmsRun test_cfg.py inputFiles=file:splashes_350968_FEVT.root nThreads=1 compressionAlgorithm=LZMA compressionLevel=9 outputFile=hcalnano.lzma9.root
echo "Done with LZMA 9"
echo "Starting LZMA 5"
time cmsRun test_cfg.py inputFiles=file:splashes_350968_FEVT.root nThreads=1 compressionAlgorithm=LZMA compressionLevel=5 outputFile=hcalnano.lzma5.root
echo "Done with LZMA 5"
echo "Starting LZMA 1"
time cmsRun test_cfg.py inputFiles=file:splashes_350968_FEVT.root nThreads=1 compressionAlgorithm=LZMA compressionLevel=1 outputFile=hcalnano.lzma1.root
echo "Done with LZMA 1"

echo "Starting ZLIB 9"
time cmsRun test_cfg.py inputFiles=file:splashes_350968_FEVT.root nThreads=1 compressionAlgorithm=ZLIB compressionLevel=9 outputFile=hcalnano.zlib9.root
echo "Done with ZLIB 9"
echo "Starting ZLIB 5"
time cmsRun test_cfg.py inputFiles=file:splashes_350968_FEVT.root nThreads=1 compressionAlgorithm=ZLIB compressionLevel=5 outputFile=hcalnano.zlib5.root
echo "Done with ZLIB 5"
echo "Starting ZLIB 1"
time cmsRun test_cfg.py inputFiles=file:splashes_350968_FEVT.root nThreads=1 compressionAlgorithm=ZLIB compressionLevel=1 outputFile=hcalnano.zlib1.root
echo "Done with ZLIB 1"

echo "Starting ZSTD 9"
time cmsRun test_cfg.py inputFiles=file:splashes_350968_FEVT.root nThreads=1 compressionAlgorithm=ZSTD compressionLevel=9 outputFile=hcalnano.zstd9.root
echo "Done with ZSTD 9"
echo "Starting ZSTD 5"
time cmsRun test_cfg.py inputFiles=file:splashes_350968_FEVT.root nThreads=1 compressionAlgorithm=ZSTD compressionLevel=5 outputFile=hcalnano.zstd5.root
echo "Done with ZSTD 5"
echo "Starting ZSTD 1"
time cmsRun test_cfg.py inputFiles=file:splashes_350968_FEVT.root nThreads=1 compressionAlgorithm=ZSTD compressionLevel=1 outputFile=hcalnano.zstd1.root
echo "Done with ZSTD 1"

echo "Starting LZ4 9"
time cmsRun test_cfg.py inputFiles=file:splashes_350968_FEVT.root nThreads=1 compressionAlgorithm=LZ4 compressionLevel=9 outputFile=hcalnano.lz49.root
echo "Done with LZ4 9"
echo "Starting LZ4 5"
time cmsRun test_cfg.py inputFiles=file:splashes_350968_FEVT.root nThreads=1 compressionAlgorithm=LZ4 compressionLevel=5 outputFile=hcalnano.lz45.root
echo "Done with LZ4 5"
echo "Starting LZ4 1"
time cmsRun test_cfg.py inputFiles=file:splashes_350968_FEVT.root nThreads=1 compressionAlgorithm=LZ4 compressionLevel=1 outputFile=hcalnano.lz41.root
echo "Done with LZ4 1"
