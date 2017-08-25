/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Linnea Ingmar <linnea.ingmar@hotmail.com>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Linnea Ingmar, 2017
 *     Christian Schulte, 2017
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

namespace Gecode { namespace Int { namespace Extensional {
      
   /*
    * Indexing for a bit-fiddling bit-set
    *
    */
   template<unsigned int sz>
   forceinline unsigned int
   SmallBitSet<sz>::Index::index(unsigned int p) const {
     assert(p < sz);
     unsigned int shift = (p+1U) << 2;
     return (data >> shift) & 15ULL;
   }
   template<unsigned int sz>
   forceinline void
   SmallBitSet<sz>::Index::index(unsigned int p, unsigned int i) {
     assert((p < sz) && (i < 16U));
     unsigned long long int li =
       static_cast<unsigned long long int>(i);
     unsigned int shift = (p+1U) << 2;
     data &= ~(15ULL << shift);
     data |= li << shift;
     assert(index(p) == i);
   }

   template<unsigned int sz>
   forceinline unsigned int
   SmallBitSet<sz>::Index::limit(void) const {
     return data & 15ULL;
   }

   template<unsigned int sz>
   forceinline void
   SmallBitSet<sz>::Index::limit(unsigned int i) {
     assert(i < 16U);
     data &= ~(15ULL);
     data |= static_cast<unsigned long long int>(i);
   }

   /*
    * Bit-fiddling bit-set
    *
    */
      
   template<unsigned int sz>
   forceinline BitSetData
   SmallBitSet<sz>::word(unsigned int p) const {
     return bits[p];
   }

   template<unsigned int sz>
   forceinline unsigned int
   SmallBitSet<sz>::index(unsigned int p) const {
     return idx.index(p);
   }

   template<unsigned int sz>
   forceinline unsigned int
   SmallBitSet<sz>::width(void) const {
     assert(!empty());
     unsigned int width = index(0);
     /// Find the largest active index
     for (unsigned int i = idx.limit(); i--; ) {
       width = std::max(width,index(i));
     }
     assert(width+1U >= words());
     return width+1U;
   }
      
   template<unsigned int sz>
   forceinline void
   SmallBitSet<sz>::replace_and_decrease(unsigned int i, BitSetData w) {
     assert(i < idx.limit());
     if (w != bits[i]) {
       bits[i] = w;
       if (w.none()) {
         assert(bits[i].none());
         bits[i] = bits[idx.limit() - 1];
         idx.index(i,idx.index(idx.limit()-1));
         idx.limit(idx.limit()-1);
       }
     }
   }
      
   template<unsigned int sz>
   forceinline unsigned int
   SmallBitSet<sz>::words(void) const {
     return idx.limit();
   }

   template<unsigned int sz>
   forceinline unsigned int
   SmallBitSet<sz>::size(void) const {
     return words();
   }
      
   template<unsigned int sz>
   forceinline
   SmallBitSet<sz>::SmallBitSet(Space&, unsigned int n) {
     assert(n <= sz);
     idx.limit(n);
     /// Set the active bits
     for (unsigned int i=0U; i < n; i++) {
       idx.index(i,i);
       assert(idx.index(i) == i);
       bits[i].init(true);
     }
     /// Clear unused suffix bits
     for (unsigned int i=n; i < sz; i++)
       bits[i].init(false);
     assert(idx.limit() == n);
   }

   template<unsigned int sz>
   template<unsigned int largersz>
   forceinline
   SmallBitSet<sz>::SmallBitSet(Space&, const SmallBitSet<largersz>& sbs) {
     assert(sz <= largersz);
     assert(sbs.idx.limit() > 0);
     idx.data = sbs.idx.data;
     for (unsigned int i=sz; i--; )
       bits[i] = sbs.bits[i];
     assert(idx.limit() <= sz);
     assert(!empty());
   }

   template<unsigned int sz>
   forceinline
   SmallBitSet<sz>::SmallBitSet(Space&, const SparseBitSet& sbs) {
     assert(sz >= sbs.words());
     idx.limit(sbs.words());
     for (unsigned int i=sbs.words(); i--; ) {
       assert(sbs.index(i) <= 15U);
       bits[i] = sbs.word(i);
       idx.index(i,sbs.index(i));
       assert(idx.index(i) == sbs.index(i));
     }
     assert(!empty());
   }
      
   template<unsigned int sz>
   template<unsigned int tinysz>
   forceinline
   SmallBitSet<sz>::SmallBitSet(Space&, const TinyBitSet<tinysz>&) {
     GECODE_NEVER;
   }
      
   template<unsigned int sz>
   forceinline bool
   SmallBitSet<sz>::empty(void) const {
     return idx.limit() == 0;
   }
   
   template<unsigned int sz>
   forceinline void
   SmallBitSet<sz>::clear_mask(BitSetData* mask) const {
     for (unsigned int i = words(); i--; )
       mask[i].init(false);
   }
      
   template<unsigned int sz>
   forceinline void
   SmallBitSet<sz>::add_to_mask(const BitSetData* b, BitSetData* mask) const {
     assert(!empty());
     for (unsigned int i = idx.limit(); i--; )
       mask[i] = BitSetData::o(mask[i],b[idx.index(i)]);
   }

   template<unsigned int sz>
   template<bool sparse>
   forceinline void
   SmallBitSet<sz>::intersect_with_mask(const BitSetData* mask) {
     assert(!empty());
     if (sparse) {
       for (unsigned int i = idx.limit(); i--; ) {
         assert(!bits[i].none());
         BitSetData w_i = bits[i];
         BitSetData w_a = BitSetData::a(w_i, mask[idx.index(i)]);
         replace_and_decrease(i,w_a);
         assert(static_cast<unsigned int>(i) == idx.limit() ||
                !bits[i].none());
       }
     } else { // The same except different indexing in mask
       for (unsigned int i = idx.limit(); i--; ) {
         assert(!bits[i].none());
         BitSetData w_i = bits[i];
         BitSetData w_a = BitSetData::a(w_i, mask[i]);
         replace_and_decrease(i,w_a);
         assert(static_cast<unsigned int>(i) == idx.limit() ||
                !bits[i].none());
       }
     }
   }
      
   template<unsigned int sz>
   forceinline void
   SmallBitSet<sz>::intersect_with_masks(const BitSetData* a, const BitSetData* b) {
     assert(!empty());
     for (unsigned int i = idx.limit(); i--; ) {
       assert(!bits[i].none());
       BitSetData w_i = bits[i];
       unsigned int offset = idx.index(i);
       BitSetData w_o = BitSetData::o(a[offset], b[offset]);
       BitSetData w_a = BitSetData::a(w_i,w_o);
       replace_and_decrease(i,w_a);
       assert(static_cast<unsigned int>(i) == idx.limit() ||
              !bits[i].none());
     }
   }
   template<unsigned int sz>
   forceinline bool
   SmallBitSet<sz>::intersects(const BitSetData* b) {
     for (unsigned int i = idx.limit(); i--; )
       if (!BitSetData::a(bits[i],b[idx.index(i)]).none())
         return true;
     return false;
   }

   template<unsigned int sz>
   forceinline void
   SmallBitSet<sz>::nand_with_mask(const BitSetData* b) {
     assert(!empty());
     for (unsigned int i = idx.limit(); i--; ) {
       assert(!bits[i].none());
       BitSetData w = BitSetData::a(bits[i],~(b[idx.index(i)]));
       replace_and_decrease(i,w);
       assert(static_cast<unsigned int>(i) == idx.limit() ||
              !bits[i].none());
     }
   }

}}}

// STATISTICS: int-prop