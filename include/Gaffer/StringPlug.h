//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2012, John Haddon. All rights reserved.
//  Copyright (c) 2011-2015, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      * Redistributions of source code must retain the above
//        copyright notice, this list of conditions and the following
//        disclaimer.
//
//      * Redistributions in binary form must reproduce the above
//        copyright notice, this list of conditions and the following
//        disclaimer in the documentation and/or other materials provided with
//        the distribution.
//
//      * Neither the name of John Haddon nor the names of
//        any other contributors to this software may be used to endorse or
//        promote products derived from this software without specific prior
//        written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#ifndef GAFFER_STRINGPLUG_H
#define GAFFER_STRINGPLUG_H

#include "Gaffer/ValuePlug.h"
#include "Gaffer/Context.h"

namespace Gaffer
{

class StringPlug : public ValuePlug
{

	public :

		typedef std::string ValueType;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( Gaffer::StringPlug, StringPlugTypeId, ValuePlug );

		StringPlug(
			const std::string &name = defaultName<StringPlug>(),
			Direction direction=In,
			const std::string &defaultValue = "",
			unsigned flags = Default,
			unsigned substitutions = Context::AllSubstitutions
		);
		virtual ~StringPlug();

		unsigned substitutions() const;

		/// Accepts only instances of StringPlug or derived classes.
		virtual bool acceptsInput( const Plug *input ) const;
		virtual PlugPtr createCounterpart( const std::string &name, Direction direction ) const;

		const std::string &defaultValue() const;

		/// \undoable
		void setValue( const std::string &value );
		/// Returns the value. See comments in TypedObjectPlug::getValue()
		/// for details of the optional precomputedHash argument - and use
		/// with care!
		std::string getValue( const IECore::MurmurHash *precomputedHash = NULL ) const;

		virtual void setFrom( const ValuePlug *other );

		virtual IECore::MurmurHash hash() const;
		/// Ensures the method above doesn't mask
		/// ValuePlug::hash( h )
		using ValuePlug::hash;

	private :

		unsigned m_substitutions;

};

IE_CORE_DECLAREPTR( StringPlug );

typedef FilteredChildIterator<PlugPredicate<Plug::Invalid, StringPlug> > StringPlugIterator;
typedef FilteredChildIterator<PlugPredicate<Plug::In, StringPlug> > InputStringPlugIterator;
typedef FilteredChildIterator<PlugPredicate<Plug::Out, StringPlug> > OutputStringPlugIterator;

typedef FilteredRecursiveChildIterator<PlugPredicate<Plug::Invalid, StringPlug>, PlugPredicate<> > RecursiveStringPlugIterator;
typedef FilteredRecursiveChildIterator<PlugPredicate<Plug::In, StringPlug>, PlugPredicate<> > RecursiveInputStringPlugIterator;
typedef FilteredRecursiveChildIterator<PlugPredicate<Plug::Out, StringPlug>, PlugPredicate<> > RecursiveOutputStringPlugIterator;

} // namespace Gaffer

#endif // GAFFER_STRINGPLUG_H
