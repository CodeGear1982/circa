// Copyright 2008 Paul Hodge

#ifndef CUTTLEFISH_TEXTURES_INCLUDED
#define CUTTLEFISH_TEXTURES_INCLUDED

GLenum get_texture_format(SDL_Surface *surface);
GLuint load_image_to_texture(std::string const& filename, circa::Term *errorListener);
GLuint load_surface_to_texture(SDL_Surface *surface);

namespace textures {

void initialize(circa::Branch& branch);

} // namespace textures

#endif