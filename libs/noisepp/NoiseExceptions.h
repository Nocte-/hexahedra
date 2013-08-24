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

#ifndef NOISEPP_EXCEPTIONS_H
#define NOISEPP_EXCEPTIONS_H

#include "NoiseStdHeaders.h"
#include "NoisePlatform.h"

namespace noisepp
{

/// Exception thrown by Noise++ in case of an error.
class Exception : public std::exception
{
	private:
		std::string mDescription;
	public:
		/// Constructor.
		Exception (const std::string &desc) : mDescription(desc)
		{}
		/// Destructor.
		~Exception() throw() {}
		/// Returns the error description.
		const std::string &getDescription () const
		{
			return mDescription;
		}
		/// Returns the error description.
		virtual const char* what() const throw ()
		{
		 	return mDescription.c_str();
		}
};

/// Thrown in case of calling an unimplemented function.
class NotImplementedException : public Exception
{
	public:
		/// Constructor.
		NotImplementedException(const std::string &func) : Exception("function not implemented: '" + func + "'")
		{}
};

/// Thrown if there are unset source modules.
class NoModuleException : public Exception
{
	public:
		/// Constructor.
		NoModuleException(const std::string &func) : Exception("source module is not set in '" + func + "'")
		{}
};

/// Thrown in case of an invalid parameter.
class ParamInvalidException : public Exception
{
	public:
		/// Constructor.
		ParamInvalidException(const std::string &paramName, const std::string &desc, const std::string &func) : Exception("invalid parameter for '" + paramName + "' in '" + ( desc.empty() ? func + "'" : func + "': " + desc ))
		{}
};

/// Thrown if you request an array element that is out of its range.
class OutOfRangeException : public Exception
{
	public:
		/// Constructor.
		OutOfRangeException(const std::string &paramName, const std::string &func) : Exception("parameter '" + paramName + "' is out of range in '" + func + "'")
		{}
};

/// Thrown in case of an error while reading a pipeline from a stream.
class ReaderException : public Exception
{
	public:
		/// Constructor.
		ReaderException(const std::string &desc) : Exception(desc)
		{}
};

#define NoiseAssert(eval, name) if (!(eval)) \
	throw ParamInvalidException(#name, std::string("assertion '") + #eval + std::string("' failed"), NOISEPP_CURRENT_FUNCTION);
#define NoiseAssertRange(param, range) if (param >= range) \
	throw OutOfRangeException(#param, NOISEPP_CURRENT_FUNCTION);
#define NoiseThrowNotImplementedException \
	throw NotImplementedException (NOISEPP_CURRENT_FUNCTION)
#define NoiseThrowNoModuleException \
	throw NoModuleException (NOISEPP_CURRENT_FUNCTION)

}

#endif // NOISEEXCEPTIONS_H
