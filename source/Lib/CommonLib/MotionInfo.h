/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2019, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     MotionInfo.h
    \brief    motion information handling classes (header)
    \todo     MvField seems to be better to be inherited from Mv
*/

#ifndef __MOTIONINFO__
#define __MOTIONINFO__

#include "CommonDef.h"
#include "Mv.h"

//! \ingroup CommonLib
//! \{

// ====================================================================================================================
// Type definition
// ====================================================================================================================

/// parameters for AMVP
struct AMVPInfo
{
  Mv       mvCand[ AMVP_MAX_NUM_CANDS_MEM ];  ///< array of motion vector predictor candidates
  unsigned numCand;                       ///< number of motion vector predictor candidates
};

struct AffineAMVPInfo
{
  Mv       mvCandLT[ AMVP_MAX_NUM_CANDS_MEM ];  ///< array of affine motion vector predictor candidates for left-top corner
  Mv       mvCandRT[ AMVP_MAX_NUM_CANDS_MEM ];  ///< array of affine motion vector predictor candidates for right-top corner
  Mv       mvCandLB[ AMVP_MAX_NUM_CANDS_MEM ];  ///< array of affine motion vector predictor candidates for left-bottom corner
  unsigned numCand;                       ///< number of motion vector predictor candidates
};

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// class for motion vector with reference index
struct MvField
{
  Mv    mv;
  int16_t refIdx;

  MvField()                                    :            refIdx( NOT_VALID ) {}
  MvField( Mv const & cMv, const int iRefIdx ) : mv( cMv ), refIdx(   iRefIdx ) {}

  void setMvField( Mv const & cMv, const int iRefIdx )
  {
    CHECK( iRefIdx == -1 && cMv != Mv(0,0), "Must not happen." );
    mv     = cMv;
    refIdx = iRefIdx;
  }

  bool operator==( const MvField& other ) const
  {
    CHECK( refIdx == -1 && mv != Mv(0,0), "Error in operator== of MvField." );
    CHECK( other.refIdx == -1 && other.mv != Mv(0,0), "Error in operator== of MvField." );
    return refIdx == other.refIdx && mv == other.mv;
  }
  bool operator!=( const MvField& other ) const
  {
    CHECK( refIdx == -1 && mv != Mv(0,0), "Error in operator!= of MvField." );
    CHECK( other.refIdx == -1 && other.mv != Mv(0,0), "Error in operator!= of MvField." );
    return refIdx != other.refIdx || mv != other.mv;
  }
};

struct MotionInfo
{
  bool     isInter;
  bool     isIBCmot;
  char     interDir;
  uint16_t   sliceIdx;
  Mv      mv     [ NUM_REF_PIC_LIST_01 ];
  int16_t   refIdx [ NUM_REF_PIC_LIST_01 ];
  uint8_t         GBiIdx;
  Mv      bv;
  MotionInfo() : isInter(false), isIBCmot(false), interDir(0), sliceIdx(0), refIdx{ NOT_VALID, NOT_VALID }, GBiIdx(0) { }
  // ensure that MotionInfo(0) produces '\x000....' bit pattern - needed to work with AreaBuf - don't use this constructor for anything else
  MotionInfo(int i) : isInter(i != 0), isIBCmot(false), interDir(0), sliceIdx(0), refIdx{ 0,         0 }, GBiIdx(0) { CHECKD(i != 0, "The argument for this constructor has to be '0'"); }

  bool operator==( const MotionInfo& mi ) const
  {
    if( isInter != mi.isInter  ) return false;
    if (isIBCmot != mi.isIBCmot) return false;
    if( isInter )
    {
      if( sliceIdx != mi.sliceIdx ) return false;
      if( interDir != mi.interDir ) return false;

      if( interDir != 2 )
      {
        if( refIdx[0] != mi.refIdx[0] ) return false;
        if( mv[0]     != mi.mv[0]     ) return false;
      }

      if( interDir != 1 )
      {
        if( refIdx[1] != mi.refIdx[1] ) return false;
        if( mv[1]     != mi.mv[1]     ) return false;
      }
    }

    return true;
  }

  bool operator!=( const MotionInfo& mi ) const
  {
    return !( *this == mi );
  }
};

class GBiMotionParam
{
  bool       m_readOnly[2][33];       // 2 RefLists, 33 RefFrams
  Mv         m_mv[2][33];
  Distortion m_dist[2][33];

  bool       m_readOnlyAffine[2][2][33];
  Mv         m_mvAffine[2][2][33][3];
  Distortion m_distAffine[2][2][33];
  int        m_mvpIdx[2][2][33];

public:

  void reset()
  {
    Mv* pMv = &(m_mv[0][0]);
    for (int ui = 0; ui < 1 * 2 * 33; ++ui, ++pMv)
    {
      pMv->set(std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::max());
    }

    Mv* pAffineMv = &(m_mvAffine[0][0][0][0]);
    for (int ui = 0; ui < 2 * 2 * 33 * 3; ++ui, ++pAffineMv)
    {
      pAffineMv->set(0, 0);
    }

    memset(m_readOnly, false, 2 * 33 * sizeof(bool));
    memset(m_dist, -1, 2 * 33 * sizeof(Distortion));
    memset(m_readOnlyAffine, false, 2 * 2 * 33 * sizeof(bool));
    memset(m_distAffine, -1, 2 * 2 * 33 * sizeof(Distortion));
    memset( m_mvpIdx, 0, 2 * 2 * 33 * sizeof( int ) );
  }

  void setReadMode(bool b, uint32_t uiRefList, uint32_t uiRefIdx) { m_readOnly[uiRefList][uiRefIdx] = b; }
  bool isReadMode(uint32_t uiRefList, uint32_t uiRefIdx) { return m_readOnly[uiRefList][uiRefIdx]; }

  void setReadModeAffine(bool b, uint32_t uiRefList, uint32_t uiRefIdx, int bP4) { m_readOnlyAffine[bP4][uiRefList][uiRefIdx] = b; }
  bool isReadModeAffine(uint32_t uiRefList, uint32_t uiRefIdx, int bP4) { return m_readOnlyAffine[bP4][uiRefList][uiRefIdx]; }

  Mv&  getMv(uint32_t uiRefList, uint32_t uiRefIdx) { return m_mv[uiRefList][uiRefIdx]; }

  void copyFrom(Mv& rcMv, Distortion uiDist, uint32_t uiRefList, uint32_t uiRefIdx)
  {
    m_mv[uiRefList][uiRefIdx] = rcMv;
    m_dist[uiRefList][uiRefIdx] = uiDist;
  }

  void copyTo(Mv& rcMv, Distortion& ruiDist, uint32_t uiRefList, uint32_t uiRefIdx)
  {
    rcMv = m_mv[uiRefList][uiRefIdx];
    ruiDist = m_dist[uiRefList][uiRefIdx];
  }

  Mv& getAffineMv(uint32_t uiRefList, uint32_t uiRefIdx, uint32_t uiAffineMvIdx, int bP4) { return m_mvAffine[bP4][uiRefList][uiRefIdx][uiAffineMvIdx]; }

  void copyAffineMvFrom(Mv(&racAffineMvs)[3], Distortion uiDist, uint32_t uiRefList, uint32_t uiRefIdx, int bP4
                        , const int mvpIdx
  )
  {
    memcpy(m_mvAffine[bP4][uiRefList][uiRefIdx], racAffineMvs, 3 * sizeof(Mv));
    m_distAffine[bP4][uiRefList][uiRefIdx] = uiDist;
    m_mvpIdx[bP4][uiRefList][uiRefIdx]     = mvpIdx;
  }

  void copyAffineMvTo(Mv acAffineMvs[3], Distortion& ruiDist, uint32_t uiRefList, uint32_t uiRefIdx, int bP4
                      , int& mvpIdx
  )
  {
    memcpy(acAffineMvs, m_mvAffine[bP4][uiRefList][uiRefIdx], 3 * sizeof(Mv));
    ruiDist = m_distAffine[bP4][uiRefList][uiRefIdx];
    mvpIdx  = m_mvpIdx[bP4][uiRefList][uiRefIdx];
  }
};
struct LutMotionCand
{
  static_vector<MotionInfo, MAX_NUM_HMVP_CANDS> lut;
  static_vector<MotionInfo, MAX_NUM_HMVP_CANDS> lutIbc;
#if !JVET_N0266_SMALL_BLOCKS
  static_vector<MotionInfo, MAX_NUM_HMVP_CANDS> lutShare;
#endif
  static_vector<MotionInfo, MAX_NUM_HMVP_CANDS> lutShareIbc;
};
#if JVET_N0329_IBC_SEARCH_IMP
struct PatentBvCand
{
  Mv m_bvCands[IBC_NUM_CANDIDATES];
  int currCnt;
};
#endif
#endif // __MOTIONINFO__
