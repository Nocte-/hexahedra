// Noise++ Library
// Copyright (c) 2008, Urs C. Hanselmann
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright notice,
//      this list of conditions and the following disclaimer in the documentation
//      and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#ifndef NOISEPP_MINIMUM_H
#define NOISEPP_MINIMUM_H

#include "NoisePipeline.h"
#include "NoiseModule.h"

namespace noisepp
{
	class MinimumElement1D : public PipelineElement1D
	{
		private:
			ElementID mLeft, mRight;
			const PipelineElement1D *mLeftPtr;
			const PipelineElement1D *mRightPtr;

		public:
			MinimumElement1D (const Pipeline1D *pipe, ElementID left, ElementID right) : mLeft(left), mRight(right)
			{
				mLeftPtr = pipe->getElement (mLeft);
				mRightPtr = pipe->getElement (mRight);
			}
			virtual Real getValue (Real x, Cache *cache) const
			{
				Real left, right;
				left = getElementValue (mLeftPtr, mLeft, x, cache);
				right = getElementValue (mRightPtr, mRight, x, cache);
				if (left < right)
					return left;
				else
					return right;
			}
	};

	class MinimumElement2D : public PipelineElement2D
	{
		private:
			ElementID mLeft, mRight;
			const PipelineElement2D *mLeftPtr;
			const PipelineElement2D *mRightPtr;

		public:
			MinimumElement2D (const Pipeline2D *pipe, ElementID left, ElementID right) : mLeft(left), mRight(right)
			{
				mLeftPtr = pipe->getElement (mLeft);
				mRightPtr = pipe->getElement (mRight);
			}
			virtual Real getValue (Real x, Real y, Cache *cache) const
			{
				Real left, right;
				left = getElementValue (mLeftPtr, mLeft, x, y, cache);
				right = getElementValue (mRightPtr, mRight, x, y, cache);
				if (left < right)
					return left;
				else
					return right;
			}
	};

	class MinimumElement3D : public PipelineElement3D
	{
		private:
			ElementID mLeft, mRight;
			const PipelineElement3D *mLeftPtr;
			const PipelineElement3D *mRightPtr;

		public:
			MinimumElement3D (const Pipeline3D *pipe, ElementID left, ElementID right) : mLeft(left), mRight(right)
			{
				mLeftPtr = pipe->getElement (mLeft);
				mRightPtr = pipe->getElement (mRight);
			}
			virtual Real getValue (Real x, Real y, Real z, Cache *cache) const
			{
				Real left, right;
				left = getElementValue (mLeftPtr, mLeft, x, y, z, cache);
				right = getElementValue (mRightPtr, mRight, x, y, z, cache);
				if (left < right)
					return left;
				else
					return right;
			}
	};

	/** Minimum module.
		Returns the minimum of the two source modules.
	*/
	class MinimumModule : public DualSourceModule<MinimumElement1D, MinimumElement2D, MinimumElement3D>
	{
		public:
			/// @copydoc noisepp::Module::getType()
			ModuleTypeId getType() const { return MODULE_MINIMUM; }
	};
};

#endif
