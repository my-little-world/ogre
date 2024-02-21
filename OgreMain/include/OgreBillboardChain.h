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

// Thanks to Vincent Cantin (karmaGfa) for the original implementation of this
// class, although it has now been mostly rewritten

#ifndef _BillboardChain_H__
#define _BillboardChain_H__

#include "OgrePrerequisites.h"

#include "OgreMovableObject.h"
#include "OgreRenderable.h"
#include "OgreResourceGroupManager.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

    /** Allows the rendering of a chain of connected billboards.

        A billboard chain operates much like a traditional billboard, i.e. its
        segments always face the camera; the difference being that instead of
        a set of disconnected quads, the elements in this class are connected
        together in a chain which must always stay in a continuous strip. This
        kind of effect is useful for creating effects such as trails, beams,
        lightning effects, etc.
    @par
        A single instance of this class can actually render multiple separate
        chain segments in a single render operation, provided they all use the
        same material. To clarify the terminology: a 'segment' is a separate 
        sub-part of the chain with its own start and end (called the 'head'
        and the 'tail'). An 'element' is a single position / colour / texcoord
        entry in a segment. You can add items to the head of a chain, and 
        remove them from the tail, very efficiently. Each segment has a max
        size, and if adding an element to the segment would exceed this size, 
        the tail element is automatically removed and re-used as the new item
        on the head.
    @par
        This class has no auto-updating features to do things like alter the
        colour of the elements or to automatically add / remove elements over
        time - you have to do all this yourself as a user of the class. 
        Subclasses can however be used to provide this kind of behaviour 
        automatically. @see RibbonTrail
    */
    class _OgreExport BillboardChain : public MovableObject, public Renderable
    {
        bool getCastsShadows(void) const override { return getCastShadows(); }
    public:

        /** Contains the data of an element of the BillboardChain.
        */
        class _OgreExport Element
        {

        public:
            Element() {}

            Element(const Vector3& position, float width, float texCoord, const ColourValue& colour,
                    const Quaternion& orientation);

            Vector3 position;
            float width;
            /// U or V texture coord depending on options
            float texCoord;
            ColourValue colour;
            /// Only used when mFaceCamera == false
            Quaternion orientation;
        };

        /** Constructor (don't use directly, use factory) 
        @param name The name to give this object
        @param maxElements The maximum number of elements per chain
        @param numberOfChains The number of separate chain segments contained in this object
        @param useTextureCoords If true, use texture coordinates from the chain elements
        @param useColours If true, use vertex colours from the chain elements
        @param dynamic If true, buffers are created with the intention of being updated
        */
        BillboardChain(const String& name, size_t maxElements = 20, size_t numberOfChains = 1, 
            bool useTextureCoords = true, bool useColours = true, bool dynamic = true);

        ~BillboardChain();

        /** Set the maximum number of chain elements per chain 
        */
        virtual void setMaxChainElements(size_t maxElements);
        /** Get the maximum number of chain elements per chain 
        */
        virtual size_t getMaxChainElements(void) const { return mMaxElementsPerChain; }
        /** Set the number of chain segments (this class can render multiple chains
            at once using the same material). 
        */
        virtual void setNumberOfChains(size_t numChains);
        /** Get the number of chain segments (this class can render multiple chains
        at once using the same material). 
        */
        virtual size_t getNumberOfChains(void) const { return mChainCount; }

        /** Sets whether texture coordinate information should be included in the
            final buffers generated.
        @note You must use either texture coordinates or vertex colour since the
            vertices have no normals and without one of these there is no source of
            colour for the vertices.
        */
        virtual void setUseTextureCoords(bool use);
        /** Gets whether texture coordinate information should be included in the
            final buffers generated.
        */
        virtual bool getUseTextureCoords(void) const { return mUseTexCoords; }

        /** The direction in which texture coordinates from elements of the
            chain are used.
        */
        enum TexCoordDirection
        {
            /// Tex coord in elements is treated as the 'u' texture coordinate
            TCD_U,
            /// Tex coord in elements is treated as the 'v' texture coordinate
            TCD_V
        };
        /** Sets the direction in which texture coords specified on each element
            are deemed to run along the length of the chain.
        @param dir The direction, default is TCD_U.
        */
        virtual void setTextureCoordDirection(TexCoordDirection dir);
        /** Gets the direction in which texture coords specified on each element
            are deemed to run.
        */
        virtual TexCoordDirection getTextureCoordDirection(void) { return mTexCoordDir; }

        /** Set the range of the texture coordinates generated across the width of
            the chain elements.
        @param start Start coordinate, default 0.0
        @param end End coordinate, default 1.0
        */
        virtual void setOtherTextureCoordRange(Real start, Real end);
        /** Get the range of the texture coordinates generated across the width of
            the chain elements.
        */
        virtual const Real* getOtherTextureCoordRange(void) const { return mOtherTexCoordRange; }

        /** Sets whether vertex colour information should be included in the
            final buffers generated.
        @note You must use either texture coordinates or vertex colour since the
            vertices have no normals and without one of these there is no source of
            colour for the vertices.
        */
        virtual void setUseVertexColours(bool use);
        /** Gets whether vertex colour information should be included in the
            final buffers generated.
        */
        virtual bool getUseVertexColours(void) const { return mUseVertexColour; }

        /** Set the auto update state

            @copydetails BillboardSet::setAutoUpdate
        */
        void setAutoUpdate(bool autoUpdate);

        /** Return the auto update state */
        bool getAutoUpdate(void) const { return mAutoUpdate; }
        
        /** Add an element to the 'head' of a chain.

            If this causes the number of elements to exceed the maximum elements
            per chain, the last element in the chain (the 'tail') will be removed
            to allow the additional element to be added.
        @param chainIndex The index of the chain
        @param billboardChainElement The details to add
        */
        virtual void addChainElement(size_t chainIndex, 
            const Element& billboardChainElement);
        /** Remove an element from the 'tail' of a chain.
        @param chainIndex The index of the chain
        */
        virtual void removeChainElement(size_t chainIndex);
        /** Update the details of an existing chain element.
        @param chainIndex The index of the chain
        @param elementIndex The element index within the chain, measured from 
            the 'head' of the chain
        @param billboardChainElement The details to set
        */
        virtual void updateChainElement(size_t chainIndex, size_t elementIndex, 
            const Element& billboardChainElement);
        /** Get the detail of a chain element.
        @param chainIndex The index of the chain
        @param elementIndex The element index within the chain, measured from
            the 'head' of the chain
        */
        virtual const Element& getChainElement(size_t chainIndex, size_t elementIndex) const;

        /** Returns the number of chain elements. */
        virtual size_t getNumChainElements(size_t chainIndex) const;

        /** Remove all elements of a given chain (but leave the chain intact). */
        virtual void clearChain(size_t chainIndex);
        /** Remove all elements from all chains (but leave the chains themselves intact). */
        virtual void clearAllChains(void);

        /** Sets whether the billboard should always be facing the camera or a custom direction
            set by each point element.

            Billboards facing the camera are useful for smoke trails, light beams, etc by
            simulating a cylinder. However, because of this property, wide trails can cause
            several artefacts unless the head is properly covered.
            Therefore, non-camera-facing billboards are much more convenient for leaving big
            trails of movement from thin objects, for example a sword swing as seen in many
            fighting games.
        @param faceCamera True to be always facing the camera (Default value: True)
        @param normalVector Only used when faceCamera == false. Must be a non-zero vector.
        This vector is the "point of reference" for each point orientation. For example,
        if normalVector is Vector3::UNIT_Z, and the point's orientation is an identity
        matrix, the segment corresponding to that point will be facing towards UNIT_Z
        This vector is internally normalized.
        */
        void setFaceCamera( bool faceCamera, const Vector3 &normalVector=Vector3::UNIT_X );

        /// Get the material name in use
        virtual const String& getMaterialName(void) const { return mMaterial->getName(); }
        /// Set the material name to use for rendering
        virtual void setMaterialName( const String& name, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );


        // Overridden members follow
        Real getSquaredViewDepth(const Camera* cam) const override;
        Real getBoundingRadius(void) const override;
        const AxisAlignedBox& getBoundingBox(void) const override;
        const MaterialPtr& getMaterial(void) const override;
        const String& getMovableType(void) const override;
        void _updateRenderQueue(RenderQueue *) override;
        void getRenderOperation(RenderOperation &) override;
        bool preRender(SceneManager* sm, RenderSystem* rsys) override;
        void getWorldTransforms(Matrix4 *) const override;
        const LightList& getLights(void) const override;
        /// @copydoc MovableObject::visitRenderables
        void visitRenderables(Renderable::Visitor* visitor, 
            bool debugRenderables = false) override;



    protected:

        /// Maximum length of each chain
        uint32 mMaxElementsPerChain;
        /// Number of chains
        uint32 mChainCount;
        /// Use texture coords?
        bool mUseTexCoords;
        /// Use vertex colour?
        bool mUseVertexColour;
        /// Tell if vertex buffer should be update automatically.
        bool mAutoUpdate;
        /// Is the vertex declaration dirty?
        bool mVertexDeclDirty;
        /// Do the buffers need recreating?
        bool mBuffersNeedRecreating;
        /// Do the bounds need redefining?
        mutable bool mBoundsDirty;
        /// Is the index buffer dirty?
        bool mIndexContentDirty;
        /// Is the vertex buffer dirty?
        bool mVertexContentDirty;
        /// AABB
        mutable AxisAlignedBox mAABB;
        /// Bounding radius
        mutable Real mRadius;
        /// Material 
        MaterialPtr mMaterial;
        /// Texture coord direction
        TexCoordDirection mTexCoordDir;
        /// Other texture coord range
        Real mOtherTexCoordRange[2];
        /// When true, the billboards always face the camera
        bool mFaceCamera;

        typedef std::vector<Element> ElementList;
        /// The list holding the chain elements
        ElementList mChainElementList;

        /** Simple struct defining a chain segment by referencing a subset of
            the preallocated buffer (which will be mMaxElementsPerChain * mChainCount
            long), by it's chain index, and a head and tail value which describe
            the current chain. The buffer subset wraps at mMaxElementsPerChain
            so that head and tail can move freely. head and tail are inclusive,
            when the chain is empty head and tail are filled with high-values.
        */
        struct ChainSegment
        {
            /// The start of this chains subset of the buffer
            size_t start;
            /// The 'head' of the chain, relative to start
            size_t head;
            /// The 'tail' of the chain, relative to start
            size_t tail;
        };
        typedef std::vector<ChainSegment> ChainSegmentList;
        ChainSegmentList mChainSegmentList;

        /// Chain segment has no elements
        static const size_t SEGMENT_EMPTY;
    private:
        /// Used when mFaceCamera == false; determines the billboard's "normal". i.e.
        /// when the orientation is identity, the billboard is perpendicular to this
        /// vector
        Vector3 mNormalBase;
        /// Camera last used to build the vertex buffer
        Camera *mVertexCameraUsed;
        /// Vertex data
        std::unique_ptr<VertexData> mVertexData;
        /// Index data (to allow multiple unconnected chains)
        std::unique_ptr<IndexData> mIndexData;

        /// Setup the STL collections
        void setupChainContainers(void);
        /// Setup vertex declaration
        virtual void setupVertexDeclaration(void);
        /// Setup buffers
        virtual void setupBuffers(void);
        /// Update the contents of the vertex buffer
        virtual void updateVertexBuffer(Camera* cam);
        /// Update the contents of the index buffer
        virtual void updateIndexBuffer(void);
        virtual void updateBoundingBox(void) const;
    };
    /** @} */
    /** @} */

} // namespace

#include "OgreHeaderSuffix.h"

#endif


