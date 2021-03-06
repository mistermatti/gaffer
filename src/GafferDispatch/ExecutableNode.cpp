//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2014, Image Engine Design Inc. All rights reserved.
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

#include "Gaffer/SubGraph.h"
#include "Gaffer/Dot.h"
#include "Gaffer/Context.h"
#include "Gaffer/ArrayPlug.h"
#include "Gaffer/Process.h"

#include "GafferDispatch/Dispatcher.h"
#include "GafferDispatch/ExecutableNode.h"

using namespace IECore;
using namespace Gaffer;
using namespace GafferDispatch;

//////////////////////////////////////////////////////////////////////////
// Task implementation
//////////////////////////////////////////////////////////////////////////

ExecutableNode::Task::Task( ConstTaskPlugPtr plug, const Gaffer::Context *context )
	:	m_plug( plug ), m_context( new Context( *context ) )
{
	Context::Scope scopedContext( m_context.get() );
	m_hash = m_plug->hash();
}

ExecutableNode::Task::Task( const Task &t ) : m_plug( t.m_plug ), m_context( t.m_context ), m_hash( t.m_hash )
{
}

ExecutableNode::Task::Task( ExecutableNodePtr n, const Context *c ) : m_plug( n->taskPlug() ), m_context( new Context( *c ) )
{
	Context::Scope scopedContext( m_context.get() );
	m_hash = m_plug->hash();
}

const ExecutableNode::TaskPlug *ExecutableNode::Task::plug() const
{
	return m_plug.get();
}

const ExecutableNode *ExecutableNode::Task::node() const
{
	return runTimeCast<const ExecutableNode>( m_plug->node() );
}

const Context *ExecutableNode::Task::context() const
{
	return m_context.get();
}

const MurmurHash ExecutableNode::Task::hash() const
{
	return m_hash;
}

bool ExecutableNode::Task::operator == ( const Task &rhs ) const
{
	return ( m_hash == rhs.m_hash );
}

bool ExecutableNode::Task::operator < ( const Task &rhs ) const
{
	return ( m_hash < rhs.m_hash );
}

//////////////////////////////////////////////////////////////////////////
// TaskPlug implementation.
//////////////////////////////////////////////////////////////////////////

namespace
{

class ExecutableNodeProcess : public Gaffer::Process
{

	public :

		ExecutableNodeProcess( const IECore::InternedString &type, const ExecutableNode::TaskPlug *plug )
			:	Process( type, plug->source<Plug>(), plug )
		{
		}

		const ExecutableNode *executableNode() const
		{
			const ExecutableNode *n = runTimeCast<const ExecutableNode>( plug()->node() );
			if( !n )
			{
				throw IECore::Exception( boost::str( boost::format( "TaskPlug \"%s\" has no ExecutableNode." ) % plug()->fullName() ) );
			}
			return n;
		}

		static InternedString hashProcessType;
		static InternedString executeProcessType;
		static InternedString executeSequenceProcessType;
		static InternedString requiresSequenceExecutionProcessType;
		static InternedString preTasksProcessType;
		static InternedString postTasksProcessType;

};

InternedString ExecutableNodeProcess::hashProcessType( "executableNode:hash" );
InternedString ExecutableNodeProcess::executeProcessType( "executableNode:execute" );
InternedString ExecutableNodeProcess::executeSequenceProcessType( "executableNode:executeSequence" );
InternedString ExecutableNodeProcess::requiresSequenceExecutionProcessType( "executableNode:requiresSequenceExecution" );
InternedString ExecutableNodeProcess::preTasksProcessType( "executableNode:preTasks" );
InternedString ExecutableNodeProcess::postTasksProcessType( "executableNode:postTasks" );

} // namespace

IE_CORE_DEFINERUNTIMETYPED( ExecutableNode::TaskPlug );

ExecutableNode::TaskPlug::TaskPlug( const std::string &name, Direction direction, unsigned flags )
	:	Plug( name, direction, flags )
{
}

bool ExecutableNode::TaskPlug::acceptsChild( const Gaffer::GraphComponent *potentialChild ) const
{
	return false;
}

bool ExecutableNode::TaskPlug::acceptsInput( const Plug *input ) const
{
	if( !Plug::acceptsInput( input ) )
	{
		return false;
	}

	if( !input )
	{
		return true;
	}

	if( input->isInstanceOf( staticTypeId() ) )
	{
		return true;
	}

	// Ideally we'd return false right now, but we must
	// provide backwards compatibility with old scripts
	// where the task plugs were just represented
	// as standard Plugs, and may have been promoted to
	// Boxes and Dots in that form.
	if( input->typeId() == Plug::staticTypeId() )
	{
		const Plug *sourcePlug = input->source<Plug>();
		if( sourcePlug->isInstanceOf( staticTypeId() ) )
		{
			return true;
		}
		const Node *sourceNode = sourcePlug->node();
		return runTimeCast<const SubGraph>( sourceNode ) || runTimeCast<const Dot>( sourceNode );
	}

	return false;

}

PlugPtr ExecutableNode::TaskPlug::createCounterpart( const std::string &name, Direction direction ) const
{
	return new TaskPlug( name, direction, getFlags() );
}

IECore::MurmurHash ExecutableNode::TaskPlug::hash() const
{
	ExecutableNodeProcess p( ExecutableNodeProcess::hashProcessType, this );
	return p.executableNode()->hash( Context::current() );
}

void ExecutableNode::TaskPlug::execute() const
{
	ExecutableNodeProcess p( ExecutableNodeProcess::executeProcessType, this );
	return p.executableNode()->execute();
}

void ExecutableNode::TaskPlug::executeSequence( const std::vector<float> &frames ) const
{
	ExecutableNodeProcess p( ExecutableNodeProcess::executeSequenceProcessType, this );
	return p.executableNode()->executeSequence( frames );
}

bool ExecutableNode::TaskPlug::requiresSequenceExecution() const
{
	ExecutableNodeProcess p( ExecutableNodeProcess::requiresSequenceExecutionProcessType, this );
	return p.executableNode()->requiresSequenceExecution();
}

void ExecutableNode::TaskPlug::preTasks( Tasks &tasks ) const
{
	ExecutableNodeProcess p( ExecutableNodeProcess::preTasksProcessType, this );
	return p.executableNode()->preTasks( Context::current(), tasks );
}

void ExecutableNode::TaskPlug::postTasks( Tasks &tasks ) const
{
	ExecutableNodeProcess p( ExecutableNodeProcess::postTasksProcessType, this );
	return p.executableNode()->postTasks( Context::current(), tasks );
}

//////////////////////////////////////////////////////////////////////////
// ExecutableNode implementation
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINERUNTIMETYPED( ExecutableNode )

size_t ExecutableNode::g_firstPlugIndex;

ExecutableNode::ExecutableNode( const std::string &name )
	:	Node( name )
{
	storeIndexOfNextChild( g_firstPlugIndex );
	addChild( new ArrayPlug( "preTasks", Plug::In, new TaskPlug( "preTask0" ) ) );
	addChild( new ArrayPlug( "postTasks", Plug::In, new TaskPlug( "postTask0" ) ) );
	addChild( new TaskPlug( "task", Plug::Out ) );

	PlugPtr dispatcherPlug = new Plug( "dispatcher", Plug::In );
	addChild( dispatcherPlug );

	Dispatcher::setupPlugs( dispatcherPlug.get() );
}

ExecutableNode::~ExecutableNode()
{
}

ArrayPlug *ExecutableNode::preTasksPlug()
{
	return getChild<ArrayPlug>( g_firstPlugIndex );
}

const ArrayPlug *ExecutableNode::preTasksPlug() const
{
	return getChild<ArrayPlug>( g_firstPlugIndex );
}

ArrayPlug *ExecutableNode::postTasksPlug()
{
	return getChild<ArrayPlug>( g_firstPlugIndex + 1 );
}

const ArrayPlug *ExecutableNode::postTasksPlug() const
{
	return getChild<ArrayPlug>( g_firstPlugIndex + 1 );
}

ExecutableNode::TaskPlug *ExecutableNode::taskPlug()
{
	return getChild<TaskPlug>( g_firstPlugIndex + 2 );
}

const ExecutableNode::TaskPlug *ExecutableNode::taskPlug() const
{
	return getChild<TaskPlug>( g_firstPlugIndex + 2 );
}

Plug *ExecutableNode::dispatcherPlug()
{
	return getChild<Plug>( g_firstPlugIndex + 3 );
}

const Plug *ExecutableNode::dispatcherPlug() const
{
	return getChild<Plug>( g_firstPlugIndex + 3 );
}

void ExecutableNode::preTasks( const Context *context, Tasks &tasks ) const
{
	for( PlugIterator cIt( preTasksPlug() ); !cIt.done(); ++cIt )
	{
		Plug *source = (*cIt)->source<Plug>();
		if( source != *cIt )
		{
			if( ExecutableNodePtr n = runTimeCast<ExecutableNode>( source->node() ) )
			{
				tasks.push_back( Task( n, context ) );
			}
		}
	}
}

void ExecutableNode::postTasks( const Context *context, Tasks &tasks ) const
{
	for( PlugIterator cIt( postTasksPlug() ); !cIt.done(); ++cIt )
	{
		Plug *source = (*cIt)->source<Plug>();
		if( source != *cIt )
		{
			if( ExecutableNodePtr n = runTimeCast<ExecutableNode>( source->node() ) )
			{
				tasks.push_back( Task( n, context ) );
			}
		}
	}
}

IECore::MurmurHash ExecutableNode::hash( const Context *context ) const
{
	IECore::MurmurHash h;
	h.append( typeId() );
	return h;
}

void ExecutableNode::execute() const
{
}

void ExecutableNode::executeSequence( const std::vector<float> &frames ) const
{
	ContextPtr context = new Context( *Context::current(), Context::Borrowed );
	Context::Scope scopedContext( context.get() );

	for ( std::vector<float>::const_iterator it = frames.begin(); it != frames.end(); ++it )
	{
		context->setFrame( *it );
		execute();
	}
}

bool ExecutableNode::requiresSequenceExecution() const
{
	return false;
}
