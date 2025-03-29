#include "../include/othelloNet.h"
#include "../include/residual_block.h"

// Constructor: create all layers
OthelloNetImpl::OthelloNetImpl(int channels, int blocks)
{
    // Initial conv: [in=2, out=channels, kernel=3, stride=1, pad=1]
    conv_in = register_module("conv_in",
        torch::nn::Conv2d(torch::nn::Conv2dOptions(2, channels, 3)
            .stride(1)
            .padding(1)
            .bias(false)));
    bn_in = register_module("bn_in", torch::nn::BatchNorm2d(channels));

    // Create 'blocks' residual blocks
    for (int i = 0; i < blocks; i++) {
        auto rb = ResidualBlock(channels);
        // store them in res_blocks and also register them
        res_blocks.push_back(register_module("res_block_" + std::to_string(i), rb));
    }

    // Policy head: conv -> BN -> linear
    conv_policy = register_module("conv_policy",
        torch::nn::Conv2d(torch::nn::Conv2dOptions(channels, 2, 1).bias(false)));
    bn_policy = register_module("bn_policy", torch::nn::BatchNorm2d(2));
    fc_policy = register_module("fc_policy", torch::nn::Linear(2 * 8 * 8, 64));

    // Value head: conv -> BN -> linear -> tanh
    conv_value = register_module("conv_value",
        torch::nn::Conv2d(torch::nn::Conv2dOptions(channels, 1, 1).bias(false)));
    bn_value = register_module("bn_value", torch::nn::BatchNorm2d(1));
    fc_value_hidden = register_module("fc_value_hidden", torch::nn::Linear(64, 64));
    fc_value = register_module("fc_value", torch::nn::Linear(64, 1));
}

// Forward pass
std::pair<torch::Tensor, torch::Tensor> OthelloNetImpl::forward(torch::Tensor x)
{
    // x shape: [N, 2, 8, 8]
    x = torch::relu(bn_in->forward(conv_in->forward(x)));
    // Pass through each residual block in turn
    for (auto& block : res_blocks) {
        x = block->forward(x);
    }

    // Policy head 
    auto p = torch::relu(bn_policy->forward(conv_policy->forward(x))); // shape [N,2,8,8]
    p = p.view({ p.size(0), -1 });  // flatten to [N,128]
    p = fc_policy->forward(p);    // shape [N,64] => raw policy logits

    // Value head
    auto v = torch::relu(bn_value->forward(conv_value->forward(x))); // shape [N,1,8,8]
    v = v.view({ v.size(0), -1 }); // flatten => [N,64]
    v = torch::relu(fc_value_hidden->forward(v)); // [N,64]
    v = torch::tanh(fc_value->forward(v));        // [N,1], in [-1,+1]

    return { p, v };
}

// Utility to encode bitboards into a [1,2,8,8] float tensor
torch::Tensor OthelloNetImpl::encode_board(uint64_t boardB, uint64_t boardW)
{
    float data[2][8][8] = {};
    for (int i = 0; i < 64; i++) {
        int r = i / 8;
        int c = i % 8;
        bool isB = ((boardB >> i) & 1ULL) != 0ULL;
        bool isW = ((boardW >> i) & 1ULL) != 0ULL;
        data[0][r][c] = isB ? 1.0f : 0.0f;
        data[1][r][c] = isW ? 1.0f : 0.0f;
    }
    // from_blob => wraps data in a tensor
    // .clone() => ensures we own a separate copy
    auto t = torch::from_blob(data, { 1, 2, 8, 8 }, torch::kFloat32).clone();
    return t;
}
