#include <gtest/gtest.h>

#include "qdltplugin.h"
#include "qdltpluginmanager.h"

TEST(QDltPluginManager, invalidationAdvancesDecodePipelineGeneration)
{
    QDltPluginManager manager;
    const std::uint64_t initialGeneration = manager.decodePipelineGeneration();

    manager.invalidateDecodePipeline();

    EXPECT_GT(manager.decodePipelineGeneration(), initialGeneration);
}

TEST(QDltPlugin, modeChangesNotifyDecodePipeline)
{
    QDltPlugin plugin;
    int notifications = 0;
    plugin.setDecodePipelineChangedCallback([&notifications]() {
        ++notifications;
    });

    plugin.setMode(QDltPlugin::ModeEnable);
    EXPECT_EQ(notifications, 1);

    plugin.setMode(QDltPlugin::ModeEnable);
    EXPECT_EQ(notifications, 1);

    plugin.setMode(QDltPlugin::ModeDisable);
    EXPECT_EQ(notifications, 2);
}
