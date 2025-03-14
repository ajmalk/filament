/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_CODEGENERATOR_H
#define TNT_FILAMENT_CODEGENERATOR_H


#include "MaterialInfo.h"
#include "UibGenerator.h"

#include <filamat/MaterialBuilder.h>

#include <filament/MaterialEnums.h>

#include <private/filament/EngineEnums.h>
#include <private/filament/SamplerInterfaceBlock.h>
#include <private/filament/BufferInterfaceBlock.h>
#include <private/filament/SubpassInfo.h>
#include <private/filament/Variant.h>

#include <backend/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Log.h>
#include <utils/sstream.h>

#include <exception>
#include <iosfwd>
#include <string>
#include <variant>

#include <stdint.h>

namespace filamat {

class UTILS_PRIVATE CodeGenerator {
    using ShaderModel = filament::backend::ShaderModel;
    using ShaderStage = filament::backend::ShaderStage;
    using FeatureLevel = filament::backend::FeatureLevel;
    using TargetApi = MaterialBuilder::TargetApi;
    using TargetLanguage = MaterialBuilder::TargetLanguage;
    using ShaderQuality = MaterialBuilder::ShaderQuality;
public:
    CodeGenerator(ShaderModel shaderModel,
            TargetApi targetApi,
            TargetLanguage targetLanguage,
            FeatureLevel featureLevel) noexcept
            : mShaderModel(shaderModel),
              mTargetApi(targetApi),
              mTargetLanguage(targetLanguage),
              mFeatureLevel(featureLevel) {
        if (targetApi == TargetApi::ALL) {
            utils::slog.e << "Must resolve target API before codegen." << utils::io::endl;
            std::terminate();
        }
    }

    filament::backend::ShaderModel getShaderModel() const noexcept { return mShaderModel; }

    // insert a separator (can be a new line)
    static utils::io::sstream& generateSeparator(utils::io::sstream& out);

    // generate prolog for the given shader
    utils::io::sstream& generateCommonProlog(utils::io::sstream& out, ShaderStage stage,
            MaterialInfo const& material, filament::Variant v) const;

    static utils::io::sstream& generateCommonEpilog(utils::io::sstream& out);

    static utils::io::sstream& generateSurfaceTypes(utils::io::sstream& out, ShaderStage stage);

    // generate common functions for the given shader
    static utils::io::sstream& generateSurfaceCommon(utils::io::sstream& out, ShaderStage stage);
    static utils::io::sstream& generatePostProcessCommon(utils::io::sstream& out, ShaderStage stage);
    static utils::io::sstream& generateSurfaceMaterial(utils::io::sstream& out, ShaderStage stage);

    static utils::io::sstream& generateSurfaceFog(utils::io::sstream& out, ShaderStage stage);

    // generate the shader's main()
    static utils::io::sstream& generateSurfaceMain(utils::io::sstream& out, ShaderStage stage);
    static utils::io::sstream& generatePostProcessMain(utils::io::sstream& out, ShaderStage stage);

    // generate the shader's code for the lit shading model
    static utils::io::sstream& generateSurfaceLit(utils::io::sstream& out, ShaderStage stage,
            filament::Variant variant, filament::Shading shading, bool customSurfaceShading);

    // generate the shader's code for the unlit shading model
    static utils::io::sstream& generateSurfaceUnlit(utils::io::sstream& out, ShaderStage stage,
            filament::Variant variant, bool hasShadowMultiplier);

    // generate the shader's code for the screen-space reflections
    static utils::io::sstream& generateSurfaceReflections(utils::io::sstream& out, ShaderStage stage);

    // generate declarations for custom interpolants
    static utils::io::sstream& generateCommonVariable(utils::io::sstream& out, ShaderStage stage,
            const MaterialBuilder::CustomVariable& variable, size_t index);

    // generate declarations for non-custom "in" variables
    utils::io::sstream& generateSurfaceShaderInputs(utils::io::sstream& out, ShaderStage stage,
            const filament::AttributeBitset& attributes, filament::Interpolation interpolation,
            MaterialBuilder::PushConstantList const& pushConstants) const;
    static utils::io::sstream& generatePostProcessInputs(utils::io::sstream& out, ShaderStage stage);

    // generate declarations for custom output variables
    utils::io::sstream& generateOutput(utils::io::sstream& out, ShaderStage stage,
            const utils::CString& name, size_t index,
            MaterialBuilder::VariableQualifier qualifier,
            MaterialBuilder::Precision precision,
            MaterialBuilder::OutputType outputType) const;

    // generate no-op shader for depth prepass
    static utils::io::sstream& generateSurfaceDepthMain(utils::io::sstream& out, ShaderStage stage);

    // generate samplers
    utils::io::sstream& generateCommonSamplers(utils::io::sstream& out,
            filament::DescriptorSetBindingPoints set,
            filament::SamplerInterfaceBlock::SamplerInfoList const& list) const;

    utils::io::sstream& generateCommonSamplers(utils::io::sstream& out,
            filament::DescriptorSetBindingPoints set,
            const filament::SamplerInterfaceBlock& sib) const {
        return generateCommonSamplers(out, set, sib.getSamplerInfoList());
    }

    // generate subpass
    static utils::io::sstream& generatePostProcessSubpass(utils::io::sstream& out,
            filament::SubpassInfo subpass);

    // generate uniforms
    utils::io::sstream& generateUniforms(utils::io::sstream& out, ShaderStage stage,
            filament::DescriptorSetBindingPoints set,
            filament::backend::descriptor_binding_t binding,
            const filament::BufferInterfaceBlock& uib) const;

    // generate buffers
    utils::io::sstream& generateBuffers(utils::io::sstream& out,
            MaterialInfo::BufferContainer const& buffers) const;

    // generate an interface block
    utils::io::sstream& generateBufferInterfaceBlock(utils::io::sstream& out, ShaderStage stage,
            filament::DescriptorSetBindingPoints set,
            filament::backend::descriptor_binding_t binding,
            const filament::BufferInterfaceBlock& uib) const;

    // generate material properties getters
    static utils::io::sstream& generateMaterialProperty(utils::io::sstream& out,
            MaterialBuilder::Property property, bool isSet);

    utils::io::sstream& generateQualityDefine(utils::io::sstream& out, ShaderQuality quality) const;

    static utils::io::sstream& generateDefine(utils::io::sstream& out, const char* name, bool value);
    static utils::io::sstream& generateDefine(utils::io::sstream& out, const char* name, uint32_t value);
    static utils::io::sstream& generateDefine(utils::io::sstream& out, const char* name, const char* string);
    static utils::io::sstream& generateIndexedDefine(utils::io::sstream& out, const char* name,
            uint32_t index, uint32_t value);

    utils::io::sstream& generateSpecializationConstant(utils::io::sstream& out,
            const char* name, uint32_t id, std::variant<int, float, bool> value) const;

    utils::io::sstream& generatePushConstants(utils::io::sstream& out,
            MaterialBuilder::PushConstantList const& pushConstants,
            size_t const layoutLocation) const;

    static utils::io::sstream& generatePostProcessGetters(utils::io::sstream& out, ShaderStage stage);
    static utils::io::sstream& generateSurfaceGetters(utils::io::sstream& out, ShaderStage stage);
    static utils::io::sstream& generateSurfaceParameters(utils::io::sstream& out, ShaderStage stage);

    static void fixupExternalSamplers(
            std::string& shader, filament::SamplerInterfaceBlock const& sib,
            FeatureLevel featureLevel) noexcept;

    // These constants must match the equivalent in MetalState.h.
    // These values represent the starting index for uniform, ssbo, and sampler group [[buffer(n)]]
    // bindings. See the chart at the top of MetalState.h.
    static constexpr uint32_t METAL_PUSH_CONSTANT_BUFFER_INDEX = 20u;
    static constexpr uint32_t METAL_DESCRIPTOR_SET_BINDING_START = 21u;
    static constexpr uint32_t METAL_DYNAMIC_OFFSET_BINDING = 25u;

    uint32_t getUniqueSamplerBindingPoint() const noexcept {
        return mUniqueSamplerBindingPoint++;
    }

    uint32_t getUniqueUboBindingPoint() const noexcept {
        return mUniqueUboBindingPoint++;
    }

    uint32_t getUniqueSsboBindingPoint() const noexcept {
        return mUniqueSsboBindingPoint++;
    }

private:
    filament::backend::Precision getDefaultPrecision(ShaderStage stage) const;
    filament::backend::Precision getDefaultUniformPrecision() const;

    utils::io::sstream& generateInterfaceFields(utils::io::sstream& out,
            utils::FixedCapacityVector<filament::BufferInterfaceBlock::FieldInfo> const& infos,
            filament::backend::Precision defaultPrecision) const;

    utils::io::sstream& generateUboAsPlainUniforms(utils::io::sstream& out, ShaderStage stage,
            const filament::BufferInterfaceBlock& uib) const;

    static const char* getUniformPrecisionQualifier(filament::backend::UniformType type,
            filament::backend::Precision precision,
            filament::backend::Precision uniformPrecision,
            filament::backend::Precision defaultPrecision) noexcept;

    // return type name of sampler  (e.g.: "sampler2D")
    char const* getSamplerTypeName(filament::backend::SamplerType type,
            filament::backend::SamplerFormat format, bool multisample) const noexcept;

    // return name of the material property (e.g.: "ROUGHNESS")
    static char const* getConstantName(MaterialBuilder::Property property) noexcept;

    static char const* getPrecisionQualifier(filament::backend::Precision precision) noexcept;

    // return type (e.g.: "vec3", "vec4", "float")
    static char const* getTypeName(UniformType type) noexcept;

    // return type name of uniform Field (e.g.: "vec3", "vec4", "float")
    static char const* getUniformTypeName(filament::BufferInterfaceBlock::FieldInfo const& info) noexcept;

    // return type name of output  (e.g.: "vec3", "vec4", "float")
    static char const* getOutputTypeName(MaterialBuilder::OutputType type) noexcept;

    // return qualifier for the specified interpolation mode
    static char const* getInterpolationQualifier(filament::Interpolation interpolation) noexcept;

    static bool hasPrecision(filament::BufferInterfaceBlock::Type type) noexcept;

    ShaderModel mShaderModel;
    TargetApi mTargetApi;
    TargetLanguage mTargetLanguage;
    FeatureLevel mFeatureLevel;
    mutable uint32_t mUniqueSamplerBindingPoint = 0;
    mutable uint32_t mUniqueUboBindingPoint = 0;
    mutable uint32_t mUniqueSsboBindingPoint = 0;
};

} // namespace filamat

#endif // TNT_FILAMENT_CODEGENERATOR_H
