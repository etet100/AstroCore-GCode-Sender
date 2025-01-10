
class Utils {
    static toRadians(degrees) {
        return glMatrix.glMatrix.toRadian(degrees);
    }

    static qIsNaN(value) {
        return isNaN(value);
    }

    static cubicInterpolate(p0, p1, p2, p3, t) {
        return p1 + 0.5 * t * (p2 - p0 + t * (2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3 + t * (3.0 * (p1 - p2) + p3 - p0)));
    }

    static grid = [];
    static SIZE = 300;
    static MAP_HEIGHT = 50.0;
    static MAP_SCALE = 0.025;
    static STEP = 1.0;

    static initBicubicInterpolation() {
        Utils.grid = [];
        for (let x = -Utils.SIZE; x < Utils.SIZE; x+=Utils.STEP) {
            const col = [];
            for (let y = -Utils.SIZE; y < Utils.SIZE; y+=Utils.STEP)
            {
                col[y] = Math.sin(y * Utils.MAP_SCALE) * Math.cos(x * Utils.MAP_SCALE) * Utils.MAP_HEIGHT;
            }
            Utils.grid[x] = col;
        }
    }

    static cubicInterpolate(p0, p1, p2, p3, t) {
        return p1 + 0.5 * t * (p2 - p0 + t * (2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3 + t * (3.0 * (p1 - p2) + p3 - p0)));
    }

    static bicubicInterpolate(x, y) {
        if (!Utils.grid.length) {
            Utils.initBicubicInterpolation();
        }

        // 'grid' to dwuwymiarowa tablica przechowująca wartości Z
        // 'x' i 'y' to współrzędne w zakresie od -STEP do STEP (wewnątrz siatki)

        // Wyznaczenie indeksów całkowitych i reszt dla X i Y
        const xInt = Math.floor(x);
        const yInt = Math.floor(y);
        const xFrac = x - xInt;
        const yFrac = y - yInt;

        // Wybieranie 4x4 siatki punktów wokół (x, y)
        let result = [];
        for (let i = -1; i <= 2; i++) {
            let row = [];
            for (let j = -1; j <= 2; j++) {
                const xi = Math.max(-Utils.SIZE, Math.min(Utils.SIZE - 1, xInt + i));
                const yi = Math.max(-Utils.SIZE, Math.min(Utils.SIZE - 1, yInt + j));
                try {
                    row.push(Utils.grid[xi][yi]);
                } catch (e) {
                    console.log("Error", xi, yi,Utils.grid[xi] );
                }
            }
            result.push(row);
        }

        // Interpolacja w kierunku X dla każdej kolumny
        let colInterpolations = [];
        for (let i = 0; i < 4; i++) {
            colInterpolations.push(Utils.cubicInterpolate(result[i][0], result[i][1], result[i][2], result[i][3], xFrac));
        }

        // Interpolacja końcowa w kierunku Y
        return Utils.cubicInterpolate(colInterpolations[0], colInterpolations[1], colInterpolations[2], colInterpolations[3], yFrac);
    }

    static ZOOMSTEP = 1.1;
    static DEFAULT_ZOOM = 100.0;
    static MIN_ZOOM = 0.2;
    static ONE_DEG_IN_RAD = 0.0174533;
    static MAGIC_ZOOM_MULTIPLIER = 1.9;
    static M_PI = Math.PI;
    static GL_FLOAT_SIZE = 4;
    static VECTOR3D_SIZE = 3 * 4;
    static VECTOR2D_SIZE = 2 * 4;
    static VERTEX_SIZE = 2 * (3 * 4) + 3 * 4;
}
const ZOOMSTEP = 1.1;
const DEFAULT_ZOOM = 1300.0;
const MIN_ZOOM = 0.2;
const ONE_DEG_IN_RAD = 0.0174533;
const MAGIC_ZOOM_MULTIPLIER = 1.9;
const M_PI = Math.PI;

const GL_FLOAT_SIZE = 4;
const VECTOR3D_SIZE = 3 * GL_FLOAT_SIZE;
const VERTEX_SIZE = 3 * VECTOR3D_SIZE;

window.toRadians = glMatrix.glMatrix.toRadian;

function qIsNaN(value) {
    return isNaN(value);
}
