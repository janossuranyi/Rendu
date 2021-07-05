#include "PostProcessStack.hpp"
#include "scene/lights/Light.hpp"
#include "scene/Sky.hpp"
#include "system/System.hpp"
#include "graphics/GPU.hpp"
#include "graphics/ScreenQuad.hpp"

PostProcessStack::PostProcessStack(const glm::vec2 & resolution) : Renderer("Post process stack"){
	const int renderWidth	= int(resolution[0]);
	const int renderHeight	= int(resolution[1]);
	const Descriptor desc = {Layout::RGB16F, Filter::LINEAR_NEAREST, Wrap::CLAMP};
	_bloomBuffer	= std::unique_ptr<Framebuffer>(new Framebuffer(renderWidth, renderHeight, desc, false, "Bloom"));
	_toneMapBuffer 	= std::unique_ptr<Framebuffer>(new Framebuffer(renderWidth, renderHeight, desc, false, "Tonemap"));
	_resultFramebuffer = std::unique_ptr<Framebuffer>(new Framebuffer(renderWidth, renderHeight, desc, false, "Postproc. result"));

	// Depth of field is performed at half resolution.
	const Descriptor dofCocDesc = {Layout::RG16F, Filter::NEAREST, Wrap::CLAMP};
	const Descriptor dofGatherDesc = {Layout::RGBA16F, Filter::LINEAR_NEAREST, Wrap::CLAMP};
	_dofCocBuffer 	= std::unique_ptr<Framebuffer>(new Framebuffer(renderWidth/2, renderHeight/2, {desc, dofCocDesc}, false, "DoF CoC"));
	_dofGatherBuffer = std::unique_ptr<Framebuffer>(new Framebuffer(renderWidth/2, renderHeight/2, dofGatherDesc, false, "DoF gather"));

	_blur		= std::unique_ptr<GaussianBlur>(new GaussianBlur(_settings.bloomRadius, 2, "Bloom"));
	_preferredFormat.push_back(desc);
	_needsDepth = false;
	_bloomProgram		= Resources::manager().getProgram2D("bloom");
	_bloomComposite 	= Resources::manager().getProgram2D("scale-texture");
	_toneMappingProgram	= Resources::manager().getProgram2D("tonemap");
	_fxaaProgram		= Resources::manager().getProgram2D("fxaa");

	_dofCocProgram = Resources::manager().getProgram2D("dof-coc");
	_dofGatherProgram = Resources::manager().getProgram2D("dof-gather");
	_dofCompositeProgram = Resources::manager().getProgram2D("dof-composite");
	checkGPUError();
}

void PostProcessStack::process(const Texture * texture, const glm::mat4 & proj, const Texture * depth, Framebuffer & framebuffer, size_t layer) {

	const glm::vec2 invRenderSize = 1.0f / glm::vec2(framebuffer.width(), framebuffer.height());

	GPU::setDepthState(false);
	GPU::setBlendState(false);
	GPU::setCullState(true, Faces::BACK);

	if(_settings.dof){
		// --- DoF pass ------
		// Compute circle of confidence along with the depth and downscaled color.
		_dofCocBuffer->bind(Framebuffer::Load::DONTCARE);
		_dofCocBuffer->setViewport();
		_dofCocProgram->use();
		_dofCocProgram->uniform("projParams", glm::vec2(proj[2][2], proj[3][2]));
		_dofCocProgram->uniform("focusDist", _settings.focusDist);
		_dofCocProgram->uniform("focusScale", _settings.focusScale);
		GPU::bindTexture(texture, 0);
		GPU::bindTexture(depth, 1);
		ScreenQuad::draw();
		// Gather from neighbor samples.
		_dofGatherBuffer->bind(Framebuffer::Load::DONTCARE);
		_dofGatherBuffer->setViewport();
		_dofGatherProgram->use();
		_dofGatherProgram->uniform("invSize", 1.0f/glm::vec2(_dofCocBuffer->width(), _dofCocBuffer->height()));
		GPU::bindTexture(_dofCocBuffer->texture(0), 0);
		GPU::bindTexture(_dofCocBuffer->texture(1), 1);
		ScreenQuad::draw();
		// Finally composite back with full res image.
		_resultFramebuffer->bind(Framebuffer::Load::DONTCARE);
		_resultFramebuffer->setViewport();
		_dofCompositeProgram->use();
		GPU::bindTexture(texture, 0);
		GPU::bindTexture(_dofGatherBuffer->texture(), 1);
		ScreenQuad::draw();
	} else {
		// Else just copy the input texture to our internal result.
		_resultFramebuffer->bind(Framebuffer::Load::DONTCARE);
		_resultFramebuffer->setViewport();
		Resources::manager().getProgram2D("passthrough-pixelperfect")->use();
		ScreenQuad::draw(texture);
	}

	if(_settings.bloom) {
		// --- Bloom selection pass ------
		_bloomBuffer->bind(Framebuffer::Load::DONTCARE);
		_bloomBuffer->setViewport();
		_bloomProgram->use();
		_bloomProgram->uniform("luminanceTh", _settings.bloomTh);
		ScreenQuad::draw(_resultFramebuffer->texture());
		
		// --- Bloom blur pass ------
		_blur->process(_bloomBuffer->texture(), *_bloomBuffer);
		
		// Add back the scene content.
		_resultFramebuffer->bind(Framebuffer::Load::LOAD);
		_resultFramebuffer->setViewport();
		GPU::setBlendState(true, BlendEquation::ADD, BlendFunction::ONE, BlendFunction::ONE);
		_bloomComposite->use();
		_bloomComposite->uniform("scale", _settings.bloomMix);
		ScreenQuad::draw(_bloomBuffer->texture());
		GPU::setBlendState(false);
		// Steps below ensures that we will always have an intermediate target.
	}

	// --- Tonemapping pass ------
	_toneMapBuffer->bind(Framebuffer::Load::DONTCARE);
	_toneMapBuffer->setViewport();
	_toneMappingProgram->use();
	_toneMappingProgram->uniform("customExposure", _settings.exposure);
	_toneMappingProgram->uniform("apply", _settings.tonemap);
	ScreenQuad::draw(_resultFramebuffer->texture());

	if(_settings.fxaa) {
		framebuffer.bind(layer, 0, Framebuffer::Load::LOAD);
		framebuffer.setViewport();
		_fxaaProgram->use();
		_fxaaProgram->uniform("inverseScreenSize", invRenderSize);
		ScreenQuad::draw(_toneMapBuffer->texture());
	} else {
		GPU::blit(*_toneMapBuffer, framebuffer, 0, layer, Filter::LINEAR);
	}

}

void PostProcessStack::updateBlurPass(){
	_blur.reset(new GaussianBlur(_settings.bloomRadius, 2, "Bloom"));
}

void PostProcessStack::resize(unsigned int width, unsigned int height) {
	const glm::ivec2 renderRes(width, height);
	_toneMapBuffer->resize(renderRes);
	_bloomBuffer->resize(renderRes);
	_resultFramebuffer->resize(renderRes);
	_dofGatherBuffer->resize(renderRes/4);
	_dofCocBuffer->resize(renderRes/4);
	checkGPUError();
}

void PostProcessStack::interface(){
	ImGui::Checkbox("DoF", &_settings.dof); ImGui::SameLine();
	ImGui::Checkbox("Bloom", &_settings.bloom);ImGui::SameLine();
	ImGui::Checkbox("Tonemap", &_settings.tonemap);ImGui::SameLine();
	ImGui::Checkbox("FXAA", &_settings.fxaa);

	if(_settings.dof) {
		ImGui::Text("DoF  "); ImGui::SameLine();
		ImGui::PushItemWidth(80);
		ImGui::SliderFloat("Dist.##DoF", &_settings.focusDist, 0.1f, 10.0f, "%.3f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_NoRoundToFormat);
		ImGui::SameLine();
		ImGui::SliderFloat("Str.##DoF", &_settings.focusScale, 1.0f, 30.0f);
		ImGui::PopItemWidth();
	}

	if(_settings.bloom) {
		ImGui::Text("Bloom"); ImGui::SameLine();
		ImGui::PushItemWidth(80);
		ImGui::SliderFloat("Th.##Bloom", &_settings.bloomTh, 0.5f, 2.0f);
		ImGui::SameLine();
		ImGui::SliderFloat("Mix##Bloom", &_settings.bloomMix, 0.0f, 1.5f);
		if(ImGui::InputInt("Rad.##Bloom", &_settings.bloomRadius, 1, 10)) {
			_settings.bloomRadius = std::max(1, _settings.bloomRadius);
			updateBlurPass();
		}
		ImGui::PopItemWidth();
	}

	if(_settings.tonemap) {
		ImGui::PushItemWidth(160);
		ImGui::SliderFloat("Exposure", &_settings.exposure, 0.1f, 10.0f);
		ImGui::PopItemWidth();
	}
}
