class Cube {
    constructor(width, height) {
        this.width = width;
        this.height = height;
        this.m_viewMatrix = glMatrix.mat4.create();
        this.m_projectionMatrix = glMatrix.mat4.create();
        this.m_rotation = glMatrix.vec3.fromValues(0, 0, Cube.DISTANCE);
        this.m_fbo = gl.createFramebuffer();
        this.m_vbo = gl.createBuffer();
        this.renderTexture = this.createTexture();
        this.m_program = new ShaderProgram(vertexShaderSource1Cube, fragmentShaderSource1Cube);
        this.setProjection();
        this.draw();
    }
    createTexture() {
        var texture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, texture);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA8, // Internal format of 
        this.width, this.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.bindTexture(gl.TEXTURE_2D, null);
        return texture;
    }
    setProjection() {
        glMatrix.mat4.identity(this.m_projectionMatrix);
        glMatrix.mat4.ortho(this.m_projectionMatrix, -10, 10, -10, 10, 1, 70);
    }
    updateView() {
        glMatrix.mat4.identity(this.m_viewMatrix);
        glMatrix.vec3.normalize(this.m_rotation, this.m_rotation);
        glMatrix.vec3.scale(this.m_rotation, this.m_rotation, Cube.DISTANCE);
        glMatrix.mat4.lookAt(this.m_viewMatrix, this.m_rotation, [0, 0, 0], [0, 1, 0]);
    }
    updateRotation(x, y, z) {
        this.m_rotation = glMatrix.vec3.fromValues(x, y, z);
        this.draw();
    }
    drawCube() {
        return;
        this.m_program.bind();
        this.updateView();
        const mvpMatrix = glMatrix.mat4.create();
        glMatrix.mat4.multiply(mvpMatrix, this.m_projectionMatrix, this.m_viewMatrix);
        this.m_program.setUniformValueMatrix("mvp_matrix", mvpMatrix);
        const cMatrix = glMatrix.mat4.create();
        glMatrix.mat4.scale(cMatrix, cMatrix, glMatrix.vec3.fromValues(0.9, 0.9, 0.9));
        this.m_program.setUniformValueMatrix("c_matrix", cMatrix);
        //
        gl.bindBuffer(gl.ARRAY_BUFFER, this.m_vbo);
        let offset = 0;
        const vertexLocation = this.m_program.attributeLocation("a_position");
        this.m_program.enableAttributeArray(vertexLocation);
        this.m_program.setAttributeBuffer(vertexLocation, gl.FLOAT, offset, 3, Utils.VERTEX_SIZE);
        offset += Utils.VECTOR3D_SIZE;
        const colorLocation = this.m_program.attributeLocation("a_color");
        this.m_program.enableAttributeArray(colorLocation);
        this.m_program.setAttributeBuffer(colorLocation, gl.FLOAT, offset, 3, Utils.VERTEX_SIZE);
        // Offset for line start point
        offset += Utils.VECTOR3D_SIZE;
        // Tell OpenGL programmable pipeline how to locate vertex line start point
        // const startLocation = this.m_program.attributeLocation("a_normal");
        // this.m_program.enableAttributeArray(startLocation);
        // this.m_program.setAttributeBuffer(startLocation, gl.FLOAT, offset, 3, Utils.VERTEX_SIZE);        
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(cube), gl.STATIC_DRAW);
        gl.enable(gl.CULL_FACE);
        gl.drawArrays(gl.TRIANGLES, 0, cube.length / 9);
        gl.disable(gl.CULL_FACE);
        this.m_program.release();
        gl.flush();
    }
    draw() {
        gl.bindFramebuffer(gl.FRAMEBUFFER, this.m_fbo);
        gl.viewport(0, 0, this.width, this.height);
        gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.renderTexture, 0);
        gl.clearColor(0, 0, 0, 0.5);
        gl.clear(gl.COLOR_BUFFER_BIT);
        this.drawCube();
        gl.flush();
        gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, null, // detach
        0);
        gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    }
}
Cube.DISTANCE = 50;
