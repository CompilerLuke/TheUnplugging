#include "frameBuffer.h"
#include <glad/glad.h>
#include "texture.h"

GLenum to_opengl(InternalColorFormat format) {
	if (format == Rgb16f) return GL_RGB16F;
}

GLenum to_opengl(ColorFormat format) {
	if (format == Rgb) return GL_RGB;
}

GLenum to_opengl(TexelType format) {
	if (format == Float_Texel) return GL_FLOAT;
}

GLenum to_opengl(Filter filter) {
	if (filter == Nearest) return GL_NEAREST;
	if (filter == Linear) return GL_LINEAR;
}

GLenum to_opengl(Wrap wrap) {
	if (wrap == ClampToBorder) return GL_CLAMP_TO_BORDER;
	if (wrap == Repeat) return GL_REPEAT;
}

AttachmentSettings::AttachmentSettings(ID id) {
	this->tex_id = id;
}

void set_texture_settings(World& world, AttachmentSettings& self, unsigned int tex) {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, to_opengl(self.min_filter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, to_opengl(self.mag_filter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, to_opengl(self.wrap_s));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, to_opengl(self.wrap_t));

	auto entity = world.make<Entity>(self.tex_id);
	entity->layermask = game_layer;

	auto texture = world.make<Texture>(self.tex_id);
	texture->texture_id = tex;
}

void add_attachment(World& world, unsigned int width, unsigned int height, AttachmentSettings& self, GLenum gl_attach) {
	unsigned int tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0, to_opengl(self.internal_format), width, height, 0, to_opengl(self.external_format), to_opengl(self.texel_type), NULL);
	set_texture_settings(world, self, tex);

	glFramebufferTexture2D(GL_FRAMEBUFFER, gl_attach, GL_TEXTURE_2D, tex, 0);
}

void add_depth_attachment(World& world, unsigned int width, unsigned int height, AttachmentSettings& self, DepthBufferSettings& depth_buffer) {
	unsigned int tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, to_opengl(self.texel_type), NULL);
	set_texture_settings(world, self, tex);

	auto entity = world.make<Entity>(self.tex_id);
	entity->layermask = game_layer;

	auto texture = world.make<Texture>(self.tex_id);
	texture->texture_id = tex;

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex, 0);
}

Framebuffer::Framebuffer(World& world, FramebufferSettings& settings) {
	unsigned int fbo;
	unsigned int rbo;

	glGenFramebuffers(1, &fbo);
	glGenRenderbuffers(1, &rbo);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);

	if (settings.depth_buffer == DepthComponent24) {
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, settings.width, settings.height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo);
	}

	for (int i = 0; i < settings.color_attachmens.size(); i++) {
		auto& attach = settings.color_attachmens[i];
		add_attachment(world, settings.width, settings.height, attach, GL_COLOR_ATTACHMENT0 + i);
	}

	if (settings.depth_attachment) {
		add_depth_attachment(world, settings.width, settings.height, *settings.depth_attachment, settings.depth_buffer);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_FRAMEBUFFER, 0);

	this->fbo = fbo;
	this->rbo = rbo;
	this->width = settings.width;
	this->height = settings.height;
}

void Framebuffer::bind() {
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void Framebuffer::clear_color(glm::vec4 color) {
	glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Framebuffer::clear_depth(glm::vec4 color) {
	glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void Framebuffer::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}