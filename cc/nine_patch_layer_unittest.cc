// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"

#include "cc/nine_patch_layer.h"

#include "cc/layer_tree_host.h"
#include "cc/occlusion_tracker.h"
#include "cc/overdraw_metrics.h"
#include "cc/rendering_stats.h"
#include "cc/resource_provider.h"
#include "cc/single_thread_proxy.h"
#include "cc/resource_update_queue.h"
#include "cc/texture_uploader.h"
#include "cc/test/fake_graphics_context.h"
#include "cc/test/fake_layer_tree_host_client.h"
#include "cc/test/geometry_test_utils.h"
#include "cc/test/layer_tree_test_common.h"
#include "SkBitmap.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using namespace cc;
using ::testing::Mock;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::AnyNumber;

namespace {

class MockLayerTreeHost : public LayerTreeHost {
public:
    MockLayerTreeHost()
        : LayerTreeHost(&m_fakeClient, LayerTreeSettings())
    {
        initialize();
    }

private:
    FakeLayerImplTreeHostClient m_fakeClient;
};


class NinePatchLayerTest : public testing::Test {
public:
    NinePatchLayerTest()
    {
    }

protected:
    virtual void SetUp()
    {
        m_layerTreeHost.reset(new MockLayerTreeHost);
    }

    virtual void TearDown()
    {
        Mock::VerifyAndClearExpectations(m_layerTreeHost.get());
    }

    scoped_ptr<MockLayerTreeHost> m_layerTreeHost;
};

TEST_F(NinePatchLayerTest, triggerFullUploadOnceWhenChangingBitmap)
{
    scoped_refptr<NinePatchLayer> testLayer = NinePatchLayer::create();
    ASSERT_TRUE(testLayer);
    testLayer->setIsDrawable(true);
    testLayer->setBounds(gfx::Size(100, 100));

    m_layerTreeHost->setRootLayer(testLayer);
    Mock::VerifyAndClearExpectations(m_layerTreeHost.get());
    EXPECT_EQ(testLayer->layerTreeHost(), m_layerTreeHost.get());

    m_layerTreeHost->initializeRendererIfNeeded();

    PriorityCalculator calculator;
    ResourceUpdateQueue queue;
    OcclusionTracker occlusionTracker(gfx::Rect(), false);
    RenderingStats stats;

    // No bitmap set should not trigger any uploads.
    testLayer->setTexturePriorities(calculator);
    testLayer->update(queue, &occlusionTracker, stats);
    EXPECT_EQ(queue.fullUploadSize(), 0);
    EXPECT_EQ(queue.partialUploadSize(), 0);

    // Setting a bitmap set should trigger a single full upload.
    SkBitmap bitmap;
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, 10, 10);
    bitmap.allocPixels();
    testLayer->setBitmap(bitmap, gfx::Rect(5, 5, 1, 1));
    testLayer->setTexturePriorities(calculator);
    testLayer->update(queue, &occlusionTracker, stats);
    EXPECT_EQ(queue.fullUploadSize(), 1);
    EXPECT_EQ(queue.partialUploadSize(), 0);
    ResourceUpdate params = queue.takeFirstFullUpload();
    EXPECT_TRUE(params.texture != NULL);

    // Upload the texture.
    m_layerTreeHost->contentsTextureManager()->setMaxMemoryLimitBytes(1024 * 1024);
    m_layerTreeHost->contentsTextureManager()->prioritizeTextures();

    scoped_ptr<GraphicsContext> context;
    scoped_ptr<ResourceProvider> resourceProvider;
    {
        DebugScopedSetImplThread implThread;
        DebugScopedSetMainThreadBlocked mainThreadBlocked;
        context = WebKit::createFakeGraphicsContext();
        resourceProvider = ResourceProvider::create(context.get());
        params.texture->acquireBackingTexture(resourceProvider.get());
        ASSERT_TRUE(params.texture->haveBackingTexture());
    }

    // Nothing changed, so no repeated upload.
    testLayer->setTexturePriorities(calculator);
    {
        DebugScopedSetImplThread implThread;
        testLayer->update(queue, &occlusionTracker, stats);
    }
    EXPECT_EQ(queue.fullUploadSize(), 0);
    EXPECT_EQ(queue.partialUploadSize(), 0);

    {
        DebugScopedSetImplThread implThread;
        DebugScopedSetMainThreadBlocked mainThreadBlocked;
        m_layerTreeHost->contentsTextureManager()->clearAllMemory(resourceProvider.get());
    }

    // Reupload after eviction
    testLayer->setTexturePriorities(calculator);
    {
        DebugScopedSetImplThread implThread;
        testLayer->update(queue, &occlusionTracker, stats);
    }
    EXPECT_EQ(queue.fullUploadSize(), 1);
    EXPECT_EQ(queue.partialUploadSize(), 0);
}

} // anonymous namespace
