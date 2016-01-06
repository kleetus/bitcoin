// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "blocksizecalculator.h"
#include "consensus.h"

using namespace BlockSizeCalculator;
using namespace std;

CChain chainActive;

unsigned int BlockSizeCalculator::ComputeBlockSize(void) {

    unsigned int proposedBlockSize = GetMedianBlockSize();
    return proposedBlockSize > MAX_BLOCK_SIZE ? proposedBlockSize : MAX_BLOCK_SIZE;
}

unsigned int GetMedianBlockSize(std::vector<unsigned int> blocksizes) {
    
    std::vector<unsigned int> blocksizes = GetBlockSizes();
    
    unsigned int vsize = blocksizes.size();
    if (vsize > 0) {
        std::sort(blocksizes.begin(), blocksizes.end());
        unsigned int median = 0;
        if ((vsize % 2) == 0) {
            median = (blocksizes[vsize/2] + blocksizes[(vsize/2) - 1])/2;
        } else {
            median = blocksizes[vsize/2];
        }
        return median;
    } else {
        return MAX_BLOCK_SIZE;
    }
}

std::vector<unsigned int> GetBlockSizes(void) {
    std::vector<unsigned int> blocksizes;
   
    unsigned int currentHeight = chainActive.GetHeight();
    unsigned int firstBlock = currentHeight - NUM_BLOCKS_FOR_MEDIAN_BLOCK;
    
    if (firstBlock > 0) {
        
        for (int i = firstBlock; i <= currentHeight; i++) {
            unsigned int blocksize = GetBlockSize(i);
            if (blocksize != -1) {
                blocksizes.push_back(blocksize);
            }
        }
            
    }
    
    return blocksizes;
}

unsigned int GetBlockSize(unsigned int height) {
    CBlockIndex* pblockindex;
    
    pblockindex = chainActive[height];
    if (pblockindex == NULL) {
        return -1;
    }
    
    const CDiskBlockPos& pos = pblockindex->GetBlockPos();
    
    CAutoFile filein(OpenBlockFile(pos, true), SER_DISK, CLIENT_VERSION);

    if (filein.IsNull()) {
        return -1;
    }
    
    FILE* blockFile = filein.release();
    long int filePos = ftell(blockFile);
    fseek(blockFile, filePos - sizeof(uint32_t), SEEK_SET);
    
    uint32_t size = 0;
    fread(&size, sizeof(uint32_t), 1, blockFile);
    return (unsigned int)size;
}
