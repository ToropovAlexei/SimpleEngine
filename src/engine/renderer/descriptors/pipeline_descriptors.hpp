
#pragma once
#include <limits>
#include <vulkan/vulkan_core.h>

namespace engine {
namespace renderer {
constexpr size_t INVALID_ID = std::numeric_limits<size_t>::max();
constexpr int MAX_RENDER_TARGETS = 8;

enum class BlendMode {
  Zero = VK_BLEND_FACTOR_ZERO,
  One = VK_BLEND_FACTOR_ONE,
  SrcColor = VK_BLEND_FACTOR_SRC_COLOR,
  InvSrcColor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
  DstColor = VK_BLEND_FACTOR_DST_COLOR,
  InvDstColor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
  SrcAlpha = VK_BLEND_FACTOR_SRC_ALPHA,
  InvSrcAlpha = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
  DstAlpha = VK_BLEND_FACTOR_DST_ALPHA,
  InvDstAlpha = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
  ConstColor = VK_BLEND_FACTOR_CONSTANT_COLOR,
  InvConstColor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
  ConstAlpha = VK_BLEND_FACTOR_CONSTANT_ALPHA,
  InvConstAlpha = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
  SrcAlphaSat = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
  Src1Color = VK_BLEND_FACTOR_SRC1_COLOR,
  InvSrc1Color = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
  Src1Alpha = VK_BLEND_FACTOR_SRC1_ALPHA,
  InvSrc1Alpha = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,
};

enum class BlendOp {
  Add = VK_BLEND_OP_ADD,
  Subtract = VK_BLEND_OP_SUBTRACT,
  RevSubtract = VK_BLEND_OP_REVERSE_SUBTRACT,
  Min = VK_BLEND_OP_MIN,
  Max = VK_BLEND_OP_MAX
};

enum class LogicOp {
  Clear = VK_LOGIC_OP_CLEAR,
  Add = VK_LOGIC_OP_AND,
  AndReverse = VK_LOGIC_OP_AND_REVERSE,
  Copy = VK_LOGIC_OP_COPY,
  AndInv = VK_LOGIC_OP_AND_INVERTED,
  NoOp = VK_LOGIC_OP_NO_OP,
  Xor = VK_LOGIC_OP_XOR,
  Or = VK_LOGIC_OP_OR,
  Nor = VK_LOGIC_OP_NOR,
  Eq = VK_LOGIC_OP_EQUIVALENT,
  Inv = VK_LOGIC_OP_INVERT,
  OrReverse = VK_LOGIC_OP_OR_REVERSE,
  CopyInv = VK_LOGIC_OP_COPY_INVERTED,
  Orinv = VK_LOGIC_OP_OR_INVERTED,
  Nand = VK_LOGIC_OP_NAND,
  Set = VK_LOGIC_OP_SET,
};

enum ColorWriteEnable {
  R = VK_COLOR_COMPONENT_R_BIT,
  G = VK_COLOR_COMPONENT_G_BIT,
  B = VK_COLOR_COMPONENT_B_BIT,
  A = VK_COLOR_COMPONENT_A_BIT,
  All = R | G | B | A
};

struct RTBlendState {
  bool blendEnable = false;
  bool logicOpEnable = false;

  BlendMode srcBlend = BlendMode::One;
  BlendMode destBlend = BlendMode::Zero;
  BlendOp blendOp = BlendOp::Add;

  BlendMode srcBlendAlpha = BlendMode::One;
  BlendMode destBlendAlpha = BlendMode::Zero;
  BlendOp blendOpAlpha = BlendOp::Add;

  LogicOp logicOp = LogicOp::NoOp;
  uint8_t renderTargetWriteMask = ColorWriteEnable::All;
};

struct BlendState {
  bool alphaToCoverageEnable = false;
  bool independentBlendEnable = false;
  RTBlendState renderTargets[MAX_RENDER_TARGETS];
};

enum class FillMode { Solid = VK_POLYGON_MODE_FILL, Wireframe = VK_POLYGON_MODE_LINE };

enum class CullMode { None = VK_CULL_MODE_NONE, Front = VK_CULL_MODE_FRONT_BIT, Back = VK_CULL_MODE_BACK_BIT };

enum class SampleCount {
  SampleCount1 = VK_SAMPLE_COUNT_1_BIT,
  SampleCount2 = VK_SAMPLE_COUNT_2_BIT,
  SampleCount4 = VK_SAMPLE_COUNT_4_BIT,
  SampleCount8 = VK_SAMPLE_COUNT_8_BIT,
  SampleCount16 = VK_SAMPLE_COUNT_16_BIT,
  SampleCount32 = VK_SAMPLE_COUNT_32_BIT,
  SampleCount64 = VK_SAMPLE_COUNT_64_BIT
};

enum class FrontFaceState { Clockwise = VK_FRONT_FACE_CLOCKWISE, CounterClockwise = VK_FRONT_FACE_COUNTER_CLOCKWISE };

struct RasterizerState {
  FillMode fillMode = FillMode::Solid;
  CullMode cullMode = CullMode::Back;
  FrontFaceState frontFaceMode = FrontFaceState::Clockwise;
  bool depthBiasEnabled = false;
  float depthBias = 0;
  float depthBiasClamp = 0.0f;
  float depthBiasSlopeFactor = 0.0f;
  SampleCount sampleCount = SampleCount::SampleCount1;
  bool depthClampEnabled = false;
};

enum class PrimitiveTopology {
  PointList = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
  LineList = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
  LineStrip = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
  TriangleList = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
  TriangleStrip = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
  TriangleFan = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
  LineListWithAdjacency = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
  LineStripWithAdjacency = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
  TriangleListWithAdjacency = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
  TriangleStripWithAdjacency = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
  PatchList = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST
};

enum class ComparisonFunc {
  Never = VK_COMPARE_OP_NEVER,
  Less = VK_COMPARE_OP_LESS,
  Equal = VK_COMPARE_OP_EQUAL,
  LessEqual = VK_COMPARE_OP_LESS_OR_EQUAL,
  Greater = VK_COMPARE_OP_GREATER,
  NotEqual = VK_COMPARE_OP_NOT_EQUAL,
  GreaterEqual = VK_COMPARE_OP_GREATER_OR_EQUAL,
  Always = VK_COMPARE_OP_ALWAYS,
};

enum class StencilOp {
  Keep = VK_STENCIL_OP_KEEP,
  Zero = VK_STENCIL_OP_ZERO,
  Replace = VK_STENCIL_OP_REPLACE,
  IncClamp = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
  DecrClamp = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
  invert = VK_STENCIL_OP_INVERT,
  IncWrap = VK_STENCIL_OP_INCREMENT_AND_WRAP,
  DecrWrap = VK_STENCIL_OP_DECREMENT_AND_WRAP,
};

struct DepthStencilOpDesc {
  StencilOp stencilFailOp = StencilOp::Keep;
  StencilOp stencilDepthFailOp = StencilOp::Keep;
  StencilOp stencilPassOp = StencilOp::Keep;
  ComparisonFunc stencilFunc = ComparisonFunc::Always;
};

struct DepthStencilState {
  bool depthEnable = false;
  bool depthWriteEnable = false;
  ComparisonFunc depthFunc = ComparisonFunc::Less;
  bool stencilEnable = false;
  uint8_t stencilReadMask = 255;
  uint8_t stencilWriteMask = 255;
  DepthStencilOpDesc frontFace;
  DepthStencilOpDesc backFace;
};

enum class InputFormat {
  Undefined = VK_FORMAT_UNDEFINED,
  // 32 bit per component
  R32G32B32A32_FLOAT = VK_FORMAT_R32G32B32A32_SFLOAT,
  R32G32B32A32_UINT = VK_FORMAT_R32G32B32A32_UINT,
  R32G32B32A32_SINT = VK_FORMAT_R32G32B32A32_SINT,
  R32G32B32_FLOAT = VK_FORMAT_R32G32B32_SFLOAT,
  R32G32B32_UINT = VK_FORMAT_R32G32B32_UINT,
  R32G32B32_SINT = VK_FORMAT_R32G32B32_SINT,
  R32G32_FLOAT = VK_FORMAT_R32G32_SFLOAT,
  R32G32_UINT = VK_FORMAT_R32G32_UINT,
  R32G32_SINT = VK_FORMAT_R32G32_SINT,
  R32_FLOAT = VK_FORMAT_R32_SFLOAT,
  R32_UINT = VK_FORMAT_R32_UINT,
  R32_SINT = VK_FORMAT_R32_SINT,
  // 16 bit per component
  R16G16B16A16_FLOAT = VK_FORMAT_R16G16B16A16_SFLOAT,
  R16G16B16A16_UINT = VK_FORMAT_R16G16B16A16_UINT,
  R16G16B16A16_SINT = VK_FORMAT_R16G16B16A16_SINT,
  R16G16_FLOAT = VK_FORMAT_R16G16_SFLOAT,
  R16G16_UINT = VK_FORMAT_R16G16_UINT,
  R16G16_SINT = VK_FORMAT_R16G16_SINT,
  R16_FLOAT = VK_FORMAT_R16_SFLOAT,
  R16_UINT = VK_FORMAT_R16_UINT,
  R16_SINT = VK_FORMAT_R16_SINT,
  // 8 bit per component
  R8G8B8A8_UNORM = VK_FORMAT_R8G8B8A8_UNORM,
  R8G8B8A8_UINT = VK_FORMAT_R8G8B8A8_UINT,
  R8G8B8A8_SINT = VK_FORMAT_R8G8B8A8_SINT,
  R8G8_UINT = VK_FORMAT_R8G8_UINT,
  R8G8_SINT = VK_FORMAT_R8G8_SINT,
  R8_UINT = VK_FORMAT_R8_UINT,
  R8_SINT = VK_FORMAT_R8_SINT,
};

enum class VertexInputRate { Vertex = VK_VERTEX_INPUT_RATE_VERTEX, Instance = VK_VERTEX_INPUT_RATE_INSTANCE };

enum class DepthImageFormat {
  Unknown = VK_FORMAT_UNDEFINED,
  // 32-bit Z w/ Stencil
  D32_FLOAT_S8X24_UINT = VK_FORMAT_D32_SFLOAT_S8_UINT,
  // No Stencil
  D32_FLOAT = VK_FORMAT_D32_SFLOAT,
  R32_FLOAT = VK_FORMAT_R32_SFLOAT,
  // 24-bit Z
  D24_UNORM_S8_UINT = VK_FORMAT_D24_UNORM_S8_UINT,
  // 16-bit Z w/o Stencil
  D16_UNORM = VK_FORMAT_D16_UNORM,
  R16_UNORM = VK_FORMAT_R16_UNORM
};

struct GraphicsPipelineDesc {
  static const int MAX_INPUT_LAYOUTS = 8;

  size_t vertexShaderId = INVALID_ID;
  size_t fragmentShaderId = INVALID_ID;
  PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList;
  RasterizerState rasterizerState;
  DepthStencilState depthStencilState;
  BlendState blendState;
  DepthImageFormat depthImageFormat = DepthImageFormat::Unknown;
};
} // namespace renderer
} // namespace engine