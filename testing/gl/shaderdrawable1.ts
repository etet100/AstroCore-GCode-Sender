declare const vertexData__: any;

class ShaderDrawable1 {
    m_needsUpdateGeometry = true;
    m_visible = true;
    m_lineWidth = 0.1;
    m_pointSize = 1.0;
    m_globalAlpha = 1.0;
    m_texture: WebGLTexture | null = null;
    m_depthTexture: WebGLTexture | null = null;
    private m_fbo: WebGLFramebuffer | null;
    private m_vbo: WebGLBuffer | null = null;
    private m_lines: Vertexes = new Vertexes();
    private m_triangles: Vertexes = new Vertexes();
    m_points = [];
    m_program: ShaderProgram = null;
    m_program2d: ShaderProgram2d = null;
    private width = 800;
    private height = 600;

    constructor(program: ShaderProgram, program2d: ShaderProgram2d) {
        Utils.initBicubicInterpolation();
        this.m_program = program;
        this.m_program2d = program2d;
        this.m_fbo = gl.createFramebuffer();
        this.m_vbo = gl.createBuffer();
        this.m_texture = this.createTexture();
        this.m_depthTexture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, this.m_depthTexture);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.DEPTH_COMPONENT32F,
            this.width, this.height, 0,
            gl.DEPTH_COMPONENT, gl.FLOAT, null);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.bindTexture(gl.TEXTURE_2D, null);
    }

    createTexture(): WebGLTexture {
        var texture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, texture);
        gl.texImage2D(
            gl.TEXTURE_2D,
            0,
            gl.RGBA8, // Internal format of
            this.width,
            this.height,
            0,
            gl.RGBA,
            gl.UNSIGNED_BYTE,
            null
        );
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.bindTexture(gl.TEXTURE_2D, null);

        return texture;
    }

    init()
    {
        // Create buffers
        //this.m_vao = gl.createVertexArray();
        this.m_vbo = gl.createBuffer();
    }

    update()
    {
        this.m_needsUpdateGeometry = true;
    }

    updateData()
    {
        // Test data
        // this.m_lines = new Vertexes();
        // this.m_lines.push(new VertexData(glMatrix.vec3.fromValues(0, 0, 0), glMatrix.vec3.fromValues(1, 0, 0), glMatrix.vec3.fromValues(NaN, 0, 0)));
        // this.m_lines.push(new VertexData(glMatrix.vec3.fromValues(10, 0, 0), glMatrix.vec3.fromValues(1, 0, 0), glMatrix.vec3.fromValues(NaN, 0, 0)));
        // this.m_lines.push(new VertexData(glMatrix.vec3.fromValues(0, 0, 0), glMatrix.vec3.fromValues(0, 1, 0), glMatrix.vec3.fromValues(NaN, 0, 0)));
        // this.m_lines.push(new VertexData(glMatrix.vec3.fromValues(0, -10, 0), glMatrix.vec3.fromValues(1, 1, 0), glMatrix.vec3.fromValues(NaN, 0, 0)));
        // this.m_lines.push(new VertexData(glMatrix.vec3.fromValues(0, 0, 0), glMatrix.vec3.fromValues(0, 0, 1), glMatrix.vec3.fromValues(NaN, 0, 0)));
        // this.m_lines.push(new VertexData(glMatrix.vec3.fromValues(0, 0, 10), glMatrix.vec3.fromValues(0, 0, 1), glMatrix.vec3.fromValues(NaN, 0, 0)));

        // Grid
        const SIZE = 50;
        // const color1 = glMatrix.vec3.fromValues(1, 1, 1);
        // const color2 = glMatrix.vec3.fromValues(0, 0, 1);

        // this.m_lines = new Vertexes();

        // for (let i = -SIZE; i < SIZE; i++) {
        //     this.m_lines.push(new VertexData(glMatrix.vec3.fromValues(i * 10, -SIZE * 10, 0), color1, glMatrix.vec3.fromValues(NaN, NaN, NaN)));
        //     this.m_lines.push(new VertexData(glMatrix.vec3.fromValues(i * 10, SIZE * 10, 0), color2, glMatrix.vec3.fromValues(NaN, NaN, NaN)));
        //     this.m_lines.push(new VertexData(glMatrix.vec3.fromValues(-SIZE * 10, i * 10, 0), color1, glMatrix.vec3.fromValues(NaN, NaN, NaN)));
        //     this.m_lines.push(new VertexData(glMatrix.vec3.fromValues(SIZE * 10, i * 10, 0), color2, glMatrix.vec3.fromValues(NaN, NaN, NaN)));
        // }

        return true;
    }

    updateGeometry()
    {
        this.m_program.bind();

        // Init in context
        if (this.m_vbo == null) this.init();

        // console.log(this.m_vao);
        //if (this.m_vao.isCreated()) {
            // Prepare vao
            //this.m_vao.bind();
        //gl.bindVertexArray(this.m_vao);
        //}

        // Prepare vbo
        gl.bindBuffer(gl.ARRAY_BUFFER, this.m_vbo);
        //this.m_vbo.bind();

        // Update vertex buffer
        if (this.updateData()) {
            // Fill vertices buffer
//            const vertexData = [];
//                        vertexData.push(...this.m_triangles);
            //vertexData.push(...this.m_lines);
//                        vertexData.push(...this.m_points);
            //
            // QVector<VertexData> vertexData(this.m_triangles);
            // vertexData += this.m_lines;
            // vertexData += this.m_points;
            //this.m_vbo.allocate(vertexData, vertexData.length * VertexData.sizeof());
            //console.log(this.m_lines);
            //gl.bufferData(gl.ARRAY_BUFFER, this.m_lines.toRawArray(), gl.STATIC_DRAW);
        } else {
            gl.bindBuffer(gl.ARRAY_BUFFER, null);
            //if (this.m_vao.isCreated()) m_vao.release();
            this.m_needsUpdateGeometry = false;
            this.m_program.release();
            return;
        }

        this.m_program.release();
        return;

        this.m_needsUpdateGeometry = false;
    }

    getMinimumExtremes() {
        return glMatrix.vec3.fromValues(-500, -500, -1);
    }

    getMaximumExtremes() {
        return glMatrix.vec3.fromValues(500, 500, 1);
    }

    getVertexCount() {
        return 0;
    }

    calculateNormal(P1, P2, P3) {
        // Oblicz wektory krawędzi
        let v1 = glMatrix.vec3.create();
        let v2 = glMatrix.vec3.create();
        glMatrix.vec3.sub(v1, P2, P1);
        glMatrix.vec3.sub(v2, P3, P1);

        // Iloczyn wektorowy
        let normal = glMatrix.vec3.create();
        glMatrix.vec3.cross(normal, v1, v2);

        // Normalizacja wektora
        glMatrix.vec3.normalize(normal, normal);

        return normal;
    }

    calculateMidNormal(N1, N2) {
        let normal = glMatrix.vec3.fromValues(0, 0, 0);
        glMatrix.vec3.add(normal, N1, N2);
        //glMatrix.vec3.scale(normal, normal, 120.5);
        glMatrix.vec3.normalize(normal, normal);
        return normal;
    }

    private once = true;

    draw(eye)
    {
        if (!this.m_visible) return;

        gl.bindFramebuffer(gl.FRAMEBUFFER, this.m_fbo);
        gl.viewport(0, 0, this.width, this.height);
        gl.lineWidth(1.3);

        gl.framebufferTexture2D(
            gl.FRAMEBUFFER,
            gl.COLOR_ATTACHMENT0,
            gl.TEXTURE_2D,
            this.m_texture,
            0
        );
        gl.framebufferTexture2D(
            gl.FRAMEBUFFER,
            gl.DEPTH_ATTACHMENT,
            gl.TEXTURE_2D,
            this.m_depthTexture,
            0
        );

        const status = gl.checkFramebufferStatus(gl.FRAMEBUFFER);
        if (status !== gl.FRAMEBUFFER_COMPLETE) {
            console.error("Framebuffer is incomplete:", status);

            throw new Error("Framebuffer is incomplete");
        }

        //gl.colorMask(false, true, true, true);
        //252, 249, 197, 255 = 1.0
        gl.clearColor(1.0, 1.0, 1.0, 1.0);
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

        this.m_program.bind();

        gl.depthMask(true);
        gl.enable(gl.DEPTH_TEST);

        // Prepare vbo
        gl.bindBuffer(gl.ARRAY_BUFFER, this.m_vbo);
        this.bindAttributes();

        const color1 = glMatrix.vec3.fromValues(1, 0, 0);
        const color2 = glMatrix.vec3.fromValues(0, 1, 0);
        const color3 = glMatrix.vec3.fromValues(0, 1, 0);
        const color4 = glMatrix.vec3.fromValues(0, 0, 0);

        this.m_triangles = new Vertexes();

        const MICROSTEP = 4;

        function computeNormals(points: Vertexes) {
            // let normals = [];
            // let ino = 0;
            let normal :any;
            let lastNormal = glMatrix.vec3.create();
            let newPoints = new Vertexes();
            for (let i = 0; i < points.length - 2; i+=2) {
                let tangent = glMatrix.vec3.create();
                glMatrix.vec3.subtract(tangent, points[i + 1].m_position, points[i].m_position);
                glMatrix.vec3.normalize(tangent, tangent);
                if (i === 0) {
                    normal = glMatrix.vec3.fromValues(0, 0, 1); // Dowolny wektor normalny dla pierwszego odcinka
                    if (glMatrix.vec3.dot(normal, tangent) !== 0) {
                        // Korekta normalnej, aby była ortogonalna do tangenta
                        glMatrix.vec3.cross(normal, tangent, [0, 1, 0]);
                        glMatrix.vec3.normalize(normal, normal);
                    }
                    //
                    points[i].m_start = normal;
                } else {
                    // Korekcja normalnej na podstawie poprzedniego kierunku
                    let projectedNormal = glMatrix.vec3.create();
                    glMatrix.vec3.cross(projectedNormal, tangent, lastNormal);
                    glMatrix.vec3.cross(normal, projectedNormal, tangent);
                    glMatrix.vec3.normalize(normal, normal);
                    //
                    points[i - 1].m_start = normal;
                    points[i].m_start = normal;

                    // newPoints.push(new VertexData(points[i - 1].m_position, color4, normal));
                    // const p3 = glMatrix.vec3.fromValues(
                    //     points[i - 1].m_position[0],
                    //     points[i - 1].m_position[1],
                    //     points[i - 1].m_position[2]
                    // );
                    // glMatrix.vec3.scaleAndAdd(p3, p3, normal, 2.0);
                    // newPoints.push(new VertexData(p3, color4, normal));
                }
                glMatrix.vec3.normalize(normal, normal);
                //normals.push(normal);

                lastNormal = glMatrix.vec3.clone(normal);
            }
            //normals.push(normal);
            //return normals;

            return newPoints;
        }

        if (this.once) {
            for (let i = 0; i < vertexData__.length; i+=9) {
                let color = [vertexData__[i + 3], vertexData__[i + 4], vertexData__[i + 5]];
                //0.564706, 0, 0.027451
                //red to dark green
                // if (color[0] == 0.564706 && color[1] == 0 && color[2] == 0.027451) {
                //     //color = [0, 0, 0];
                //     color = [0, 1, 1];
                // }

                this.m_lines.push(new VertexData(
                    [vertexData__[i], vertexData__[i + 1], vertexData__[i + 2]],
                    Math.floor(Math.random() * 5),
                    [vertexData__[i + 6], vertexData__[i + 7], vertexData__[i + 8]]
                ));
            }
            const newPoints = computeNormals(this.m_lines);

            //this.m_lines.push(...newPoints);
            // console.log(normals.length, this.m_lines.length);
            // let i = 0;
            // for (const normal of normals) {
            //     this.m_lines[i].m_start = normal;
            //     this.m_lines[i+1].m_start = normal;
            //     i += 2;
            // }

//            console.log(normals);

            //console.log(vertexData);


            // for (let i = -Utils.SIZE; i <= Utils.SIZE; i += MICROSTEP) {
            //     for (let j = -Utils.SIZE; j <= Utils.SIZE; j += MICROSTEP) {
            //         const z1 = Utils.bicubicInterpolate(i, j);
            //         const z2 = Utils.bicubicInterpolate(i + MICROSTEP, j);

            //         const p1 = glMatrix.vec3.fromValues(i, j, z1);
            //         const p2 = glMatrix.vec3.fromValues(i + (MICROSTEP), j, z2);

            //         const direction = glMatrix.vec3.create();
            //         glMatrix.vec3.sub(direction, p2, p1);

            //         let normal = glMatrix.vec3.create();
            //         if (direction[2] == 0) {
            //             normal = glMatrix.vec3.fromValues(0.0, -1.0, 0.1);
            //         } else {
            //             normal = glMatrix.vec3.fromValues(-direction[2], 0.0, direction[0]);
            //         }
            //         glMatrix.vec3.normalize(normal, normal);

            //         this.m_lines.push(new VertexData([...p1], [...color1], normal));
            //         this.m_lines.push(new VertexData([...p2], [...color1], normal));

            //         // normals
            //         this.m_lines.push(new VertexData(p1, color1, normal));
            //         const p3 = glMatrix.vec3.fromValues(p1[0], p1[1], p1[2]);
            //         glMatrix.vec3.scaleAndAdd(p3, p1, normal, 2.0);
            //         this.m_lines.push(new VertexData(p3, color1, normal));
            //     }
            // }
        }

        this.once = false;

        let time = performance.now();
        const ra = this.m_lines.toRawArray();
        console.log("To raw array: ", performance.now() - time);

        gl.bufferData(gl.ARRAY_BUFFER,
            new Float32Array(ra), gl.STATIC_DRAW);

        //console.log(ra.length);

        // this.m_program.setUniformValue("u_shadow", true);
        // gl.drawArrays(gl.LINES, 0, this.m_lines.length);

        this.m_program.setUniformValue("u_shadow", false);
        gl.drawArrays(gl.LINES, 0, this.m_lines.length);

        gl.bindBuffer(gl.ARRAY_BUFFER, null);
        gl.bindFramebuffer(gl.FRAMEBUFFER, null);

        this.m_program.release();

        this.m_program2d.bind();
        this.m_program2d.setGcodeBgMode(true);

        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, this.m_texture);
        gl.texParameterf(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        this.m_program2d.setUniformValuei("u_texture", 0);

        gl.activeTexture(gl.TEXTURE1);
        gl.bindTexture(gl.TEXTURE_2D, this.m_depthTexture);
        this.m_program2d.setUniformValuei("u_depthTexture", 1);

        gl.activeTexture(gl.TEXTURE0);

        // draw to screen

        const LEFT = -1;
        const RIGHT = 1;
        const TOP = 1;
        const BOTTOM = -1;

        // 2d quad vertices
        const vertices2d = [
            LEFT, TOP,
            0, 1,
            RIGHT, TOP,
            1, 1,
            LEFT, BOTTOM,
            0, 0,
            LEFT, BOTTOM,
            0, 0,
            RIGHT, TOP,
            1, 1,
            RIGHT, BOTTOM,
            1, 0,
        ];

        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices2d), gl.STATIC_DRAW);

        // Draw texture
        gl.viewport(0, 0, 800, 600);
        // gl.disable(gl.DEPTH_TEST);
        gl.depthMask(true);
        gl.depthFunc(gl.ALWAYS);
        gl.drawArrays(gl.TRIANGLES, 0, 6);
        // gl.enable(gl.DEPTH_TEST);

        gl.bindTexture(gl.TEXTURE_2D, null);

        this.m_program2d.release();

        this.m_program.bind();

        gl.depthFunc(gl.LESS);

        gl.bindBuffer(gl.ARRAY_BUFFER, this.m_vbo);
        this.bindAttributes();

        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(ra), gl.STATIC_DRAW);

        this.m_program.setUniformValue("u_shadow", false);
        gl.drawArrays(gl.LINES, 0, this.m_lines.length);

        this.m_program.release();
    }

    private bindAttributes() {
        // Offset for position
        //quintptr

        let offset = 0;

        // Tell OpenGL programmable pipeline how to locate vertex position data
        const vertexLocation = this.m_program.attributeLocation("a_position");
        this.m_program.enableAttributeArray(vertexLocation);
        this.m_program.setAttributeBuffer(vertexLocation, gl.FLOAT, offset, 3, Utils.VERTEX_SIZE);

        // Offset for color
        offset += Utils.VECTOR3D_SIZE;

        // Tell OpenGL programmable pipeline how to locate vertex color data
        const colorLocation = this.m_program.attributeLocation("a_color");
        this.m_program.enableAttributeArray(colorLocation);
        this.m_program.setAttributeBuffer(colorLocation, gl.FLOAT, offset, 3, Utils.GL_FLOAT_SIZE);

        // Offset for line start point
        offset += Utils.VECTOR3D_SIZE; //GL_FLOAT_SIZE;

        // Tell OpenGL programmable pipeline how to locate vertex line start point
        const startLocation = this.m_program.attributeLocation("a_normal");
        this.m_program.enableAttributeArray(startLocation);
        this.m_program.setAttributeBuffer(startLocation, gl.FLOAT, offset, 3, Utils.VERTEX_SIZE);
    }

    needsUpdateGeometry() {
        return this.m_needsUpdateGeometry;
    }

    visible() {
        return true;
    }
}
