#include <gtest/gtest.h>
#include <vector>
#include <utility>
#include <algorithm>

#include <entities/Soldier.h>
#include <world/TurnQueue.h>


TEST(TurnQueueTest, GoodRotation1) {
    Soldier charac1{"soldier1", {0,0}, {1,1,1,1,1,1,1}, {}, {}};
    Soldier charac2{"soldier2", {1,3}, {1,1,1,1,1,1,1}, {}, {}};
    Soldier charac3{"soldier3", {2,2}, {1,1,1,1,1,1,1}, {}, {}};
    Soldier charac4{"soldier4", {4,1}, {1,1,1,1,1,1,1}, {}, {}};
    

    std::vector<std::pair<Character*, float>> vec = {
        {&charac1, 3.5f},
        {&charac2, 1.2f},
        {&charac3, 4.8f},
        {&charac4, 2.0f}
    };

    TurnQueue turnManager;
    turnManager.SetQueue(vec);
    ASSERT_EQ(turnManager.GetSize(), 4u);

    EXPECT_EQ(turnManager.GetCurrentCharacter()->getNameId(), "soldier2");
    turnManager.UpdateCurrentCharacter(10.0f);
    turnManager.EndCurrentCharacter();
    
    EXPECT_EQ(turnManager.GetCurrentCharacter()->getNameId(), "soldier4");
    turnManager.UpdateCurrentCharacter(10.0f);
    turnManager.EndCurrentCharacter();

    EXPECT_EQ(turnManager.GetCurrentCharacter()->getNameId(), "soldier1");
    turnManager.UpdateCurrentCharacter(10.0f);
    turnManager.EndCurrentCharacter();

    EXPECT_EQ(turnManager.GetCurrentCharacter()->getNameId(), "soldier3");
    turnManager.UpdateCurrentCharacter(10.0f);
    turnManager.EndCurrentCharacter();

    EXPECT_EQ(turnManager.GetCurrentCharacter()->getNameId(), "soldier2");
}

TEST(TurnQueueTest, NegativeAV) {
    Soldier charac1{"soldier1", {0,0}, {1,1,1,1,1,1,1}, {}, {}};
    Soldier charac2{"soldier2", {1,3}, {1,1,1,1,1,1,1}, {}, {}};
    Soldier charac3{"soldier3", {2,2}, {1,1,1,1,1,1,1}, {}, {}};
    Soldier charac4{"soldier4", {4,1}, {1,1,1,1,1,1,1}, {}, {}};
    

    std::vector<std::pair<Character*, float>> vec = {
        {&charac1, 3.5f},
        {&charac2, -2.0f},
        {&charac3, 4.8f},
        {&charac4, -1.2f}
    };

    TurnQueue turnManager;
    turnManager.SetQueue(vec);
    ASSERT_EQ(turnManager.GetSize(), 4u);


    EXPECT_EQ(turnManager.GetCurrentCharacter()->getNameId(), "soldier2");
    turnManager.UpdateCurrentCharacter(-10.0f);
    turnManager.EndCurrentCharacter();

    EXPECT_EQ(turnManager.GetCurrentCharacter()->getNameId(), "soldier2");
    turnManager.UpdateCurrentCharacter(10.0f);
    turnManager.EndCurrentCharacter();

    EXPECT_EQ(turnManager.GetCurrentCharacter()->getNameId(), "soldier2");
}
