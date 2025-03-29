#ifndef RESIDUAL_BLOCK_H
#define RESIDUAL_BLOCK_H

#include <torch/torch.h>

// A standard AlphaZero-style residual block.
// Input -> [Conv -> BN -> ReLU -> Conv -> BN] + skip -> ReLU
struct ResidualBlockImpl : torch::nn::Module {
    torch::nn::Conv2d conv1{ nullptr }, conv2{ nullptr };
    torch::nn::BatchNorm2d bn1{ nullptr }, bn2{ nullptr };

    // Constructor
    explicit ResidualBlockImpl(int channels) {
        // conv1: [channels, channels, kernel=3, padding=1]
        conv1 = register_module(
            "conv1",
            torch::nn::Conv2d(torch::nn::Conv2dOptions(channels, channels, /*kernel_size=*/3)
                .padding(1)
                .bias(false)));
        bn1 = register_module("bn1", torch::nn::BatchNorm2d(channels));

        // conv2: [channels, channels, kernel=3, padding=1]
        conv2 = register_module(
            "conv2",
            torch::nn::Conv2d(torch::nn::Conv2dOptions(channels, channels, /*kernel_size=*/3)
                .padding(1)
                .bias(false)));
        bn2 = register_module("bn2", torch::nn::BatchNorm2d(channels));
    }

    // Forward pass
    torch::Tensor forward(const torch::Tensor& x) {
        // First conv -> BN -> ReLU
        auto out = torch::relu(bn1->forward(conv1->forward(x)));
        // Second conv -> BN
        out = bn2->forward(conv2->forward(out));
        // Skip connection + ReLU
        return torch::relu(out + x);
    }
};

TORCH_MODULE(ResidualBlock);

#endif
