// Copyright (c) 2014-2024, The Monero Project
// Copyright (c) 2014-2015 The Boolberry developers
// Copyright (c) 2012-2013 The Cryptonote developers
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "gtest/gtest.h"

#include "cryptonote_basic/cryptonote_basic_impl.h"
#include "cryptonote_config.h"

using namespace cryptonote;

namespace
{
  // Test case for first values of block reward and already generated coins
  class block_reward_and_already_generated_coins_handles_first_values_Test : public ::testing::Test
  {
  protected:
    void do_test(uint64_t already_generated_coins, uint64_t expected)
    {
      m_block_reward = 0;
      m_block_not_too_big = get_block_reward(0, current_block_weight, already_generated_coins, m_block_reward, 1, 1);
      ASSERT_TRUE(m_block_not_too_big);
      ASSERT_EQ(expected, m_block_reward);
    }

    static const size_t current_block_weight = 10000;
    bool m_block_not_too_big;
    uint64_t m_block_reward;
  };

#define TEST_ALREADY_GENERATED_COINS(already_generated_coins, expected) \
  do_test(already_generated_coins, expected)

  TEST_F(block_reward_and_already_generated_coins_handles_first_values_Test, handles_first_values)
  {
    TEST_ALREADY_GENERATED_COINS(0, UINT64_C(17592186044415));
    TEST_ALREADY_GENERATED_COINS(m_block_reward, UINT64_C(17592169267200));
    TEST_ALREADY_GENERATED_COINS(UINT64_C(2756434948434199641), UINT64_C(14963444829249));
  }


  // Test case for block reward and already generated coins stepping from 2 to 1
  class block_reward_and_already_generated_coins_correctly_steps_from_2_to_1_Test : public block_reward_and_already_generated_coins_handles_first_values_Test
  {
  };

  TEST_F(block_reward_and_already_generated_coins_correctly_steps_from_2_to_1_Test, correctly_steps_from_2_to_1)
  {
    TEST_ALREADY_GENERATED_COINS(MONEY_SUPPLY - ((2 << 20) + 1), FINAL_SUBSIDY_PER_MINUTE);
    TEST_ALREADY_GENERATED_COINS(MONEY_SUPPLY -  (2 << 20)      , FINAL_SUBSIDY_PER_MINUTE);
    TEST_ALREADY_GENERATED_COINS(MONEY_SUPPLY - ((2 << 20) - 1), FINAL_SUBSIDY_PER_MINUTE);
  }


  // Test case for block reward and already generated coins behavior around max
  class block_reward_and_already_generated_coins_handles_max_Test : public block_reward_and_already_generated_coins_handles_first_values_Test
  {
  };

  TEST_F(block_reward_and_already_generated_coins_handles_max_Test, handles_max)
  {
    TEST_ALREADY_GENERATED_COINS(MONEY_SUPPLY - ((1 << 20) + 1), FINAL_SUBSIDY_PER_MINUTE);
    TEST_ALREADY_GENERATED_COINS(MONEY_SUPPLY -  (1 << 20)      , FINAL_SUBSIDY_PER_MINUTE);
    TEST_ALREADY_GENERATED_COINS(MONEY_SUPPLY - ((1 << 20) - 1), FINAL_SUBSIDY_PER_MINUTE);
  }


  class block_reward_and_current_block_weight : public ::testing::Test
  {
  protected:
    virtual void SetUp()
    {
      m_block_not_too_big = get_block_reward(0, 0, already_generated_coins, m_standard_block_reward, 1, 1);
      ASSERT_TRUE(m_block_not_too_big);
    }

    void do_test(size_t median_block_weight, size_t current_block_weight)
    {
      m_block_reward = 0;
      m_block_not_too_big = get_block_reward(median_block_weight, current_block_weight, already_generated_coins, m_block_reward, 1, 1);
    }

    static const uint64_t already_generated_coins = 0;
    uint64_t m_standard_block_reward;
    uint64_t m_block_reward;
    bool m_block_not_too_big;
  };

  TEST_F(block_reward_and_current_block_weight, handles_block_size_less_or_equal_to_median)
  {
    do_test(100000, 100000);
    ASSERT_TRUE(m_block_not_too_big);
    ASSERT_EQ(m_standard_block_reward, m_block_reward);

    do_test(100000, 10000);
    ASSERT_TRUE(m_block_not_too_big);
    ASSERT_EQ(m_standard_block_reward, m_block_reward);

    do_test(100000, 0);
    ASSERT_TRUE(m_block_not_too_big);
    ASSERT_EQ(m_standard_block_reward, m_block_reward);
  }

  TEST_F(block_reward_and_current_block_weight, handles_block_size_gt_median)
  {
    do_test(100000, 100001);
    ASSERT_TRUE(m_block_not_too_big);
    ASSERT_NE(m_standard_block_reward, m_block_reward);
    ASSERT_GT(m_standard_block_reward, m_block_reward);

    do_test(100000, 199999);
    ASSERT_TRUE(m_block_not_too_big);
    ASSERT_NE(m_standard_block_reward, m_block_reward);
    ASSERT_GT(m_standard_block_reward, m_block_reward);

    do_test(100000, 150000);
    ASSERT_TRUE(m_block_not_too_big);
    uint64_t expected_reward = m_standard_block_reward * (200000 - 150000) * 150000 / (100000ULL * 100000ULL);
    ASSERT_EQ(expected_reward, m_block_reward);
  }

  TEST_F(block_reward_and_current_block_weight, handles_block_size_gt_2_times_median)
  {
    do_test(100000, 200000);
    ASSERT_TRUE(m_block_not_too_big);
    ASSERT_EQ(0, m_block_reward);

    do_test(100000, 200001);
    ASSERT_FALSE(m_block_not_too_big);
  }


  class block_reward_and_last_block_weights : public ::testing::Test
  {
  protected:
    virtual void SetUp()
    {
      m_last_block_weights.resize(CRYPTONOTE_REWARD_BLOCKS_WINDOW);
      for (size_t i = 0; i < m_last_block_weights.size(); ++i)
      {
        m_last_block_weights[i] = 100000;
      }

      m_block_not_too_big = get_block_reward(epee::misc_utils::median(m_last_block_weights), 0, already_generated_coins, m_standard_block_reward, 1, 1);
      ASSERT_TRUE(m_block_not_too_big);
    }

    void do_test(size_t current_block_weight)
    {
      m_block_reward = 0;
      m_block_not_too_big = get_block_reward(epee::misc_utils::median(m_last_block_weights), current_block_weight, already_generated_coins, m_block_reward, 1, 1);
    }

    static const uint64_t already_generated_coins = 0;
    std::vector<size_t> m_last_block_weights;
    uint64_t m_standard_block_reward;
    uint64_t m_block_reward;
    bool m_block_not_too_big;
  };

  TEST_F(block_reward_and_last_block_weights, handles_block_size_less_or_equal_to_median)
  {
    do_test(100000);
    ASSERT_TRUE(m_block_not_too_big);
    ASSERT_EQ(m_standard_block_reward, m_block_reward);

    do_test(10000);
    ASSERT_TRUE(m_block_not_too_big);
    ASSERT_EQ(m_standard_block_reward, m_block_reward);

    do_test(0);
    ASSERT_TRUE(m_block_not_too_big);
    ASSERT_EQ(m_standard_block_reward, m_block_reward);
  }

  TEST_F(block_reward_and_last_block_weights, handles_block_size_gt_median)
  {
    do_test(100001);
    ASSERT_TRUE(m_block_not_too_big);
    ASSERT_NE(m_standard_block_reward, m_block_reward);
    ASSERT_GT(m_standard_block_reward, m_block_reward);

    do_test(199999);
    ASSERT_TRUE(m_block_not_too_big);
    ASSERT_NE(m_standard_block_reward, m_block_reward);
    ASSERT_GT(m_standard_block_reward, m_block_reward);

    do_test(150000);
    ASSERT_TRUE(m_block_not_too_big);
    uint64_t expected_reward = m_standard_block_reward * (200000 - 150000) * 150000 / (100000ULL * 100000ULL);
    ASSERT_EQ(expected_reward, m_block_reward);
  }

  TEST_F(block_reward_and_last_block_weights, handles_block_size_gt_2_times_median)
  {
    do_test(200000);
    ASSERT_TRUE(m_block_not_too_big);
    ASSERT_EQ(0, m_block_reward);

    do_test(200001);
    ASSERT_FALSE(m_block_not_too_big);
  }
}
