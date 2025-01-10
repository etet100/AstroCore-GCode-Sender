
class VertexData {
    m_position;
    m_color;
    m_start; // = m_normal

    constructor(position, color, start) {
        this.m_position = position;
        this.m_color = color;
        this.m_start = start;
    }
}

class Vertexes extends Array {
    constructor(data) {
        super();

        if (data == null) {
            return;
        }
        for (a of data) {
            this.push(a);
        }

        console.log("Vertexes", this);
    }

    pushVertex(position, color, start) {
        this.push(new VertexData(position, color, start));
    }

    toRawArray() {
        const raw = new Array(this.length * 9);
        let index = 0;
        for (const vertex of this) {
            //console.log(vertex);
            raw[index++] = vertex.m_position[0];
            raw[index++] = vertex.m_position[1];
            raw[index++] = vertex.m_position[2];
            raw[index++] = vertex.m_color;
            raw[index++] = vertex.m_color;
            raw[index++] = vertex.m_color;
            // raw[index++] = vertex.m_color[1];
            // raw[index++] = vertex.m_color[2];
            raw[index++] = vertex.m_start[0];
            raw[index++] = vertex.m_start[1];
            raw[index++] = vertex.m_start[2];
        }
        return raw;
    }
}
