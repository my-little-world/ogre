/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
  -----------------------------------------------------------------------------
*/

#include "OgreMetalHardwareBufferManager.h"
#include "OgreMetalHardwareBufferCommon.h"
#include "OgreMetalDiscardBufferManager.h"

namespace Ogre {
    MetalHardwareBufferManager::MetalHardwareBufferManager( MetalDevice *device) :
        mDiscardBufferManager( 0 )
    {
        mDiscardBufferManager = OGRE_NEW MetalDiscardBufferManager( device );
    }
    //-----------------------------------------------------------------------------------
    MetalHardwareBufferManager::~MetalHardwareBufferManager()
    {
        destroyAllDeclarations();
        destroyAllBindings();

        OGRE_DELETE mDiscardBufferManager;
        mDiscardBufferManager = 0;
    }
    //-----------------------------------------------------------------------------------
    void MetalHardwareBufferManager::_notifyDeviceStalled(void)
    {
        {
            OGRE_LOCK_MUTEX(mVertexBuffersMutex);
            VertexBufferList::const_iterator itor = mVertexBuffers.begin();
            VertexBufferList::const_iterator end  = mVertexBuffers.end();

            while( itor != end )
            {
                auto hwBuffer = (*itor)->_getImpl<MetalHardwareBufferCommon>();
                hwBuffer->_notifyDeviceStalled();
                ++itor;
            }
        }
        {
            OGRE_LOCK_MUTEX(mIndexBuffersMutex);
            IndexBufferList::const_iterator itor = mIndexBuffers.begin();
            IndexBufferList::const_iterator end  = mIndexBuffers.end();

            while( itor != end )
            {
                auto hwBuffer = (*itor)->_getImpl<MetalHardwareBufferCommon>();
                hwBuffer->_notifyDeviceStalled();
                ++itor;
            }
        }

        mDiscardBufferManager->_notifyDeviceStalled();
    }
    //-----------------------------------------------------------------------------------
    HardwareVertexBufferSharedPtr
    MetalHardwareBufferManager::createVertexBuffer( size_t vertexSize, size_t numVerts,
                                                        HardwareBuffer::Usage usage,
                                                        bool useShadowBuffer )
    {
        auto impl = new MetalHardwareBufferCommon(vertexSize * numVerts, usage, useShadowBuffer, 4, mDiscardBufferManager,
                                                  mDiscardBufferManager->getDevice());
        auto buf = std::make_shared<HardwareVertexBuffer>(this, vertexSize, numVerts, impl);
        {
            OGRE_LOCK_MUTEX(mVertexBuffersMutex);
            mVertexBuffers.insert(buf.get());
        }
        return buf;
    }
    //-----------------------------------------------------------------------------------
    HardwareIndexBufferSharedPtr
    MetalHardwareBufferManager::createIndexBuffer( HardwareIndexBuffer::IndexType itype,
                                                       size_t numIndexes,
                                                       HardwareBuffer::Usage usage,
                                                       bool useShadowBuffer )
    {
        auto indexSize = HardwareIndexBuffer::indexSize(itype);
        auto impl = new MetalHardwareBufferCommon(indexSize * numIndexes, usage, useShadowBuffer, 4, mDiscardBufferManager,
                                                  mDiscardBufferManager->getDevice());

        auto buf = std::make_shared<HardwareIndexBuffer>(this, itype, numIndexes, impl);
        {
            OGRE_LOCK_MUTEX(mIndexBuffersMutex);
            mIndexBuffers.insert(buf.get());
        }
        return buf;
    }
}