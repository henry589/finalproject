#ifndef OTHELLONET_H
#define OTHELLONET_H

#include <torch/torch.h>
#include <utility>    // for std::pair
#include <cstdint>    // for uint64_t
#include "../include/residual_block.h"
// Forward declaration of the implementation
struct OthelloNetImpl;

// This alias lets us do: OthelloNet net(...);
using OthelloNet = torch::nn::ModuleHolder<OthelloNetImpl>;

// The actual class that holds the modules
struct OthelloNetImpl : torch::nn::Module {
    // Constructor: specify channels and residual blocks
    OthelloNetImpl(int channels = 128, int blocks = 10);

    // Forward pass: returns (policy_logits, value)
    //  shape(policy_logits) = [N, 64], shape(value) = [N, 1]
    std::pair<torch::Tensor, torch::Tensor> forward(torch::Tensor x);

    // Utility to encode board into [1,2,8,8] float
    static torch::Tensor encode_board(uint64_t boardB, uint64_t boardW);

    // If want to store the layers publicly (e.g. for manual weight init),
    // can do that here:
    torch::nn::Conv2d conv_in{ nullptr };
    torch::nn::BatchNorm2d bn_in{ nullptr };

    // We'll store the residual blocks in a std::vector
    std::vector<ResidualBlock> res_blocks;

    torch::nn::Conv2d conv_policy{ nullptr };
    torch::nn::BatchNorm2d bn_policy{ nullptr };
    torch::nn::Linear fc_policy{ nullptr };

    torch::nn::Conv2d conv_value{ nullptr };
    torch::nn::BatchNorm2d bn_value{ nullptr };
    torch::nn::Linear fc_value_hidden{ nullptr };
    torch::nn::Linear fc_value{ nullptr };
};

#endif
