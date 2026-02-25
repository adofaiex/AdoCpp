#include "Tile.h"
#include <map>
// #include <boost/geometry.hpp>
// #include <earcut.hpp>

static constexpr float PI = 3.14159265358979323846, outline = 0.02f;

static bool pointIsInsideTriangle(const std::vector<sf::Vector2f>& points, const sf::Vector2f point)
{
    //////////////////////////////////////////////////////////////////////////////
    //
    // Selba Ward (https://github.com/Hapaxia/SelbaWard)
    // --
    //
    // Polygon
    //
    // Copyright(c) 2022-2025 M.J.Silk
    //
    // This software is provided 'as-is', without any express or implied
    // warranty. In no event will the authors be held liable for any damages
    // arising from the use of this software.
    //
    // Permission is granted to anyone to use this software for any purpose,
    // including commercial applications, and to alter it and redistribute it
    // freely, subject to the following restrictions :
    //
    // 1. The origin of this software must not be misrepresented; you must not
    // claim that you wrote the original software.If you use this software
    // in a product, an acknowledgment in the product documentation would be
    // appreciated but is not required.
    //
    // 2. Altered source versions must be plainly marked as such, and must not be
    // misrepresented as being the original software.
    //
    // 3. This notice may not be removed or altered from any source distribution.
    //
    // M.J.Silk
    // MJSilk2@gmail.com
    //
    //////////////////////////////////////////////////////////////////////////////
    const long double point1X{(points[0].x)};
    const long double point1Y{(points[0].y)};
    const long double point2X{(points[1].x)};
    const long double point2Y{(points[1].y)};
    const long double point3X{(points[2].x)};
    const long double point3Y{(points[2].y)};
    const long double pointX{(point.x)};
    const long double pointY{(point.y)};

    const long double denominatorMultiplier{
        1.l / ((point2Y - point3Y) * (point1X - point3X) + (point3X - point2X) * (point1Y - point3Y))};
    const long double a{((point2Y - point3Y) * (pointX - point3X) + (point3X - point2X) * (pointY - point3Y)) *
                        denominatorMultiplier};
    const long double b{((point3Y - point1Y) * (pointX - point3X) + (point1X - point3X) * (pointY - point3Y)) *
                        denominatorMultiplier};
    const long double c{1.l - a - b};
    return a >= 0.l && a <= 1.l && b >= 0.l && b <= 1.l && c >= 0.l && c <= 1.l;
}

// Thanks for StArray's code
static void createCircle(const sf::Vector3f center, const float r, std::vector<sf::Vector3f>& vertices,
                         std::vector<size_t>& m_triangles, const uint32_t resolution = 32)
{
    const size_t centerIndex = vertices.size();
    vertices.push_back(center);

    for (int i = 0; i < resolution; i++)
    {
        const float angle = 2.f * PI * static_cast<float>(i) / static_cast<float>(resolution);
        sf::Vector3f vertex = sf::Vector3f(std::cos(angle) * r, std::sin(angle) * r, 0) + center;
        vertices.push_back(vertex);
    }

    for (int i = 1; i < resolution; i++)
    {
        m_triangles.push_back(centerIndex);
        m_triangles.push_back(centerIndex + i);
        m_triangles.push_back(centerIndex + i + 1);
    }

    // Closing the circle by connecting the last vertex to the first
    m_triangles.push_back(centerIndex);
    m_triangles.push_back(centerIndex + resolution);
    m_triangles.push_back(centerIndex + 1);
}

// Thanks for StArray's code
// ReSharper disable CppDFAConstantParameter
static void createTileMesh(float width, float length, sf::Angle startAngle, sf::Angle endAngle,
                           // ReSharper restore CppDFAConstantParameter
                           const uint32_t m_interpolationLevel, sf::VertexArray& m_fillVertices,
                           sf::VertexArray& m_outlineVertices)
{
    startAngle = startAngle.wrapUnsigned(), endAngle = endAngle.wrapUnsigned();
    std::vector<sf::Vector3f> fillVertices, outlineVertices;
    std::vector<size_t> fillTriangles, outlineTriangles;

    // region basic process
    const float m11 = std::cos(startAngle.asRadians()), m12 = std::sin(startAngle.asRadians()),
                m21 = std::cos(endAngle.asRadians()), m22 = std::sin(endAngle.asRadians());
    float a[2]{};

    if ((startAngle - endAngle).wrapUnsigned() >= (endAngle - startAngle).wrapUnsigned())
    {
        a[0] = startAngle.wrapUnsigned().asRadians();
        a[1] = a[0] + (endAngle - startAngle).wrapUnsigned().asRadians();
    }
    else
    {
        a[0] = endAngle.wrapUnsigned().asRadians();
        a[1] = a[0] + (startAngle - endAngle).wrapUnsigned().asRadians();
    }
    float angle = a[1] - a[0], mid = a[0] + angle / 2.f;
    // endregion
    if (angle < 2.0943952f && angle > 0)
    {
        // region angle < 2.0943952
        float x;
        if (angle < 0.08726646f)
            x = 1.f;
        else if (angle < 0.5235988f)
            x = std::lerp(1.f, 0.83f, std::pow((angle - 0.08726646f) / 0.43633235f, 0.5f));
        else if (angle < 0.7853982f)
            x = std::lerp(0.83f, 0.77f, std::pow((angle - 0.5235988f) / 0.2617994f, 1.f));
        else if (angle < 1.5707964f)
            x = std::lerp(0.77f, 0.15f, std::pow((angle - 0.7853982f) / 0.7853982f, 0.7f));
        else
            x = std::lerp(0.15f, 0.f, std::pow((angle - 1.5707964f) / 0.5235988f, 0.5f));
        float distance;
        float radius;
        if (x == 1.f)
            radius = width, distance = 0.f;
        else
            radius = std::lerp(0.f, width, x), distance = (width - radius) / std::sin(angle / 2.f);

        float circlex = -distance * std::cos(mid), circley = -distance * std::sin(mid);
        // endregion
        // region outline
        width += outline;
        length += outline;
        radius += outline;
        createCircle(sf::Vector3f(circlex, circley, 0), radius, outlineVertices, outlineTriangles,
                     m_interpolationLevel);
        {
            const size_t count = outlineVertices.size();
            outlineVertices.emplace_back(-radius * std::sin(a[1]) + circlex, radius * std::cos(a[1]) + circley, 0);
            outlineVertices.emplace_back(circlex, circley, 0);
            outlineVertices.emplace_back(radius * std::sin(a[0]) + circlex, -radius * std::cos(a[0]) + circley, 0);
            outlineVertices.emplace_back(width * std::sin(a[0]), -width * std::cos(a[0]), 0);
            outlineVertices.emplace_back();
            outlineVertices.emplace_back(-width * std::sin(a[1]), width * std::cos(a[1]), 0);
            outlineTriangles.push_back(count);
            outlineTriangles.push_back(count + 1);
            outlineTriangles.push_back(count + 5);
            outlineTriangles.push_back(count + 4);
            outlineTriangles.push_back(count + 1);
            outlineTriangles.push_back(count + 5);
            outlineTriangles.push_back(count + 2);
            outlineTriangles.push_back(count + 3);
            outlineTriangles.push_back(count + 4);
            outlineTriangles.push_back(count + 1);
            outlineTriangles.push_back(count + 3);
            outlineTriangles.push_back(count + 4);
        }
        {
            const size_t count = outlineVertices.size();
            outlineVertices.emplace_back(length * m11 + width * m12, length * m12 - width * m11, 0);
            outlineVertices.emplace_back(length * m11 - width * m12, length * m12 + width * m11, 0);
            outlineVertices.emplace_back(-width * m12, width * m11, 0);
            outlineVertices.emplace_back(width * m12, -width * m11, 0);

            outlineVertices.emplace_back(length * m21 + width * m22, length * m22 - width * m21, 0);
            outlineVertices.emplace_back(length * m21 - width * m22, length * m22 + width * m21, 0);
            outlineVertices.emplace_back(-width * m22, width * m21, 0);
            outlineVertices.emplace_back(width * m22, -width * m21, 0);
            outlineTriangles.push_back(count);
            outlineTriangles.push_back(count + 1);
            outlineTriangles.push_back(count + 2);
            outlineTriangles.push_back(count + 2);
            outlineTriangles.push_back(count + 3);
            outlineTriangles.push_back(count);
            outlineTriangles.push_back(count + 4);
            outlineTriangles.push_back(count + 5);
            outlineTriangles.push_back(count + 6);
            outlineTriangles.push_back(count + 6);
            outlineTriangles.push_back(count + 7);
            outlineTriangles.push_back(count + 4);
        }
        // endregion
        // region fill
        width -= outline * 2.f;
        length -= outline * 2.f;
        radius -= outline * 2.f;
        if (radius < 0)
        {
            radius = 0;
            circlex = -width / std::sin(angle / 2.f) * std::cos(mid);
            circley = -width / std::sin(angle / 2.f) * std::sin(mid);
        }
        createCircle(sf::Vector3f(circlex, circley, 0), radius, fillVertices, fillTriangles, m_interpolationLevel);
        {
            const size_t count = fillVertices.size();
            fillVertices.emplace_back(-radius * std::sin(a[1]) + circlex, radius * std::cos(a[1]) + circley, 0);
            fillVertices.emplace_back(circlex, circley, 0);
            fillVertices.emplace_back(radius * std::sin(a[0]) + circlex, -radius * std::cos(a[0]) + circley, 0);
            fillVertices.emplace_back(width * std::sin(a[0]), -width * std::cos(a[0]), 0);
            fillVertices.emplace_back();
            fillVertices.emplace_back(-width * std::sin(a[1]), width * std::cos(a[1]), 0);
            fillTriangles.push_back(count);
            fillTriangles.push_back(count + 1);
            fillTriangles.push_back(count + 5);
            fillTriangles.push_back(count + 4);
            fillTriangles.push_back(count + 1);
            fillTriangles.push_back(count + 5);
            fillTriangles.push_back(count + 2);
            fillTriangles.push_back(count + 3);
            fillTriangles.push_back(count + 4);
            fillTriangles.push_back(count + 1);
            fillTriangles.push_back(count + 3);
            fillTriangles.push_back(count + 4);
        }
        {
            const size_t count = fillVertices.size();
            fillVertices.emplace_back(length * m11 + width * m12, length * m12 - width * m11, 0);
            fillVertices.emplace_back(length * m11 - width * m12, length * m12 + width * m11, 0);
            fillVertices.emplace_back(-width * m12, width * m11, 0);
            fillVertices.emplace_back(width * m12, -width * m11, 0);

            fillVertices.emplace_back(length * m21 + width * m22, length * m22 - width * m21, 0);
            fillVertices.emplace_back(length * m21 - width * m22, length * m22 + width * m21, 0);
            fillVertices.emplace_back(-width * m22, width * m21, 0);
            fillVertices.emplace_back(width * m22, -width * m21, 0);
            fillTriangles.push_back(count);
            fillTriangles.push_back(count + 1);
            fillTriangles.push_back(count + 2);
            fillTriangles.push_back(count + 2);
            fillTriangles.push_back(count + 3);
            fillTriangles.push_back(count);
            fillTriangles.push_back(count + 4);
            fillTriangles.push_back(count + 5);
            fillTriangles.push_back(count + 6);
            fillTriangles.push_back(count + 6);
            fillTriangles.push_back(count + 7);
            fillTriangles.push_back(count + 4);
        }
        // endregion
    }
    else if (angle > 0)
    {
        // region normal case
        width += outline;
        length += outline;

        float circlex = -width / std::sin(angle / 2.f) * std::cos(mid);
        float circley = -width / std::sin(angle / 2.f) * std::sin(mid);

        {
            constexpr size_t count = 0;
            outlineVertices.emplace_back(circlex, circley, 0);
            outlineVertices.emplace_back(width * std::sin(a[0]), -width * std::cos(a[0]), 0);
            outlineVertices.emplace_back();
            outlineVertices.emplace_back(-width * std::sin(a[1]), width * std::cos(a[1]), 0);
            outlineTriangles.push_back(count);
            outlineTriangles.push_back(count + 1);
            outlineTriangles.push_back(count + 2);
            outlineTriangles.push_back(count + 2);
            outlineTriangles.push_back(count + 3);
            outlineTriangles.push_back(count);
        }
        {
            const size_t count = outlineVertices.size();
            outlineVertices.emplace_back(length * m11 + width * m12, length * m12 - width * m11, 0);
            outlineVertices.emplace_back(length * m11 - width * m12, length * m12 + width * m11, 0);
            outlineVertices.emplace_back(-width * m12, width * m11, 0);
            outlineVertices.emplace_back(width * m12, -width * m11, 0);

            outlineVertices.emplace_back(length * m21 + width * m22, length * m22 - width * m21, 0);
            outlineVertices.emplace_back(length * m21 - width * m22, length * m22 + width * m21, 0);
            outlineVertices.emplace_back(-width * m22, width * m21, 0);
            outlineVertices.emplace_back(width * m22, -width * m21, 0);
            outlineTriangles.push_back(count);
            outlineTriangles.push_back(count + 1);
            outlineTriangles.push_back(count + 2);
            outlineTriangles.push_back(count + 2);
            outlineTriangles.push_back(count + 3);
            outlineTriangles.push_back(count);
            outlineTriangles.push_back(count + 4);
            outlineTriangles.push_back(count + 5);
            outlineTriangles.push_back(count + 6);
            outlineTriangles.push_back(count + 6);
            outlineTriangles.push_back(count + 7);
            outlineTriangles.push_back(count + 4);
        }
        // endregion
        // region fill
        width -= outline * 2.f;
        length -= outline * 2.f;

        circlex = -width / std::sin(angle / 2.f) * std::cos(mid);
        circley = -width / std::sin(angle / 2.f) * std::sin(mid);

        {
            const size_t count = fillVertices.size();
            fillVertices.emplace_back(circlex, circley, 0);
            fillVertices.emplace_back(width * std::sin(a[0]), -width * std::cos(a[0]), 0);
            fillVertices.emplace_back();
            fillVertices.emplace_back(-width * std::sin(a[1]), width * std::cos(a[1]), 0);
            fillTriangles.push_back(count);
            fillTriangles.push_back(count + 1);
            fillTriangles.push_back(count + 2);
            fillTriangles.push_back(count + 2);
            fillTriangles.push_back(count + 3);
            fillTriangles.push_back(count);
        }
        {
            const size_t count = fillVertices.size();
            fillVertices.emplace_back(length * m11 + width * m12, length * m12 - width * m11, 0);
            fillVertices.emplace_back(length * m11 - width * m12, length * m12 + width * m11, 0);
            fillVertices.emplace_back(-width * m12, width * m11, 0);
            fillVertices.emplace_back(width * m12, -width * m11, 0);

            fillVertices.emplace_back(length * m21 + width * m22, length * m22 - width * m21, 0);
            fillVertices.emplace_back(length * m21 - width * m22, length * m22 + width * m21, 0);
            fillVertices.emplace_back(-width * m22, width * m21, 0);
            fillVertices.emplace_back(width * m22, -width * m21, 0);
            fillTriangles.push_back(count);
            fillTriangles.push_back(count + 1);
            fillTriangles.push_back(count + 2);
            fillTriangles.push_back(count + 2);
            fillTriangles.push_back(count + 3);
            fillTriangles.push_back(count);
            fillTriangles.push_back(count + 4);
            fillTriangles.push_back(count + 5);
            fillTriangles.push_back(count + 6);
            fillTriangles.push_back(count + 6);
            fillTriangles.push_back(count + 7);
            fillTriangles.push_back(count + 4);
        }
        // endregion
    }
    else
    {
        // region outline (if included angle == 180, draw a semicircle)
        length = width;
        width += outline;
        length += outline;

        const sf::Vector3f midpoint{-m11 * 0.04f, -m12 * 0.04f, 0};
        createCircle(midpoint, width, outlineVertices, outlineTriangles, m_interpolationLevel);

        {
            const size_t count = outlineVertices.size();
            outlineVertices.push_back(midpoint +
                                      sf::Vector3f(length * m11 + width * m12, length * m12 - width * m11, 0));
            outlineVertices.push_back(midpoint +
                                      sf::Vector3f(length * m11 - width * m12, length * m12 + width * m11, 0));
            outlineVertices.push_back(midpoint + sf::Vector3f(-width * m12, width * m11, 0));
            outlineVertices.push_back(midpoint + sf::Vector3f(width * m12, -width * m11, 0));

            outlineTriangles.push_back(count);
            outlineTriangles.push_back(count + 1);
            outlineTriangles.push_back(count + 2);
            outlineTriangles.push_back(count + 2);
            outlineTriangles.push_back(count + 3);
            outlineTriangles.push_back(count);
        }
        // endregion
        // region fill
        width -= outline * 2.f;
        length -= outline * 2.f;
        createCircle(midpoint, width, fillVertices, fillTriangles, m_interpolationLevel);
        {
            const size_t count = fillVertices.size();
            fillVertices.push_back(midpoint + sf::Vector3f(length * m11 + width * m12, length * m12 - width * m11, 0));
            fillVertices.push_back(midpoint + sf::Vector3f(length * m11 - width * m12, length * m12 + width * m11, 0));
            fillVertices.push_back(midpoint + sf::Vector3f(-width * m12, width * m11, 0));
            fillVertices.push_back(midpoint + sf::Vector3f(width * m12, -width * m11, 0));

            fillTriangles.push_back(count);
            fillTriangles.push_back(count + 1);
            fillTriangles.push_back(count + 2);
            fillTriangles.push_back(count + 2);
            fillTriangles.push_back(count + 3);
            fillTriangles.push_back(count);
        }
        // endregion
    }

    // using namespace bg = boost::geometry;
    // using point_t = bg::model::point<double, 2, bg::cs::cartesian>;
    // using polygon_t = bg::model::polygon<point_t>;
    // using mpolygon_t = bg::model::multi_polygon<polygon_t>;
    // mpolygon_t fillPolygon, outlinePolygon;
    //
    // for (size_t i = 0; i < fillVertices.size(); i += 3)
    // {
    //     const size_t idx1 = fillTriangles[i], idx2 = fillTriangles[i + 1], idx3 = fillTriangles[i + 2];
    //     const sf::Vector3f v1 = fillVertices[idx1], v2 = fillVertices[idx2], v3 = fillVertices[idx3];
    //     polygon_t polygon({{v1.x, v1.y},{v2.x, v2.y},{v3.x, v3.y}});
    //     if (i != 0)
    //     {
    //         mpolygon_t tmp;
    //         bg::union_(fillPolygon, polygon, tmp);
    //         fillPolygon = tmp;
    //     }
    // }
    // for (size_t i = 0; i < outlineVertices.size(); i += 3)
    // {
    //     const size_t idx1 = outlineTriangles[i], idx2 = outlineTriangles[i + 1], idx3 = outlineTriangles[i + 2];
    //     const sf::Vector3f v1 = outlineVertices[idx1], v2 = outlineVertices[idx2], v3 = outlineVertices[idx3];
    //     polygon_t polygon({{v1.x, v1.y},{v2.x, v2.y},{v3.x, v3.y}});
    //     if (i != 0)
    //     {
    //         mpolygon_t tmp;
    //         bg::union_(outlinePolygon, polygon, tmp);
    //         outlinePolygon = tmp;
    //     }
    // }
    // {
    //     mpolygon_t tmp;
    //     bg::difference(outlinePolygon, fillPolygon, tmp);
    //     outlinePolygon = tmp;
    // }
    // {
    //
    // }
    // using Coord = double;
    //
    // using N = uint32_t;
    //
    // using Point = std::array<Coord, 2>;
    // std::vector<std::vector<Point>> polygon;
    //
    // // Fill polygon structure with actual data. Any winding order works.
    // // The first polyline defines the main polygon.
    // polygon.push_back({{100, 0}, {100, 100}, {0, 100}, {0, 0}});
    // // Following polylines define holes.
    // polygon.push_back({{75, 25}, {75, 75}, {25, 75}, {25, 25}});
    //
    // // Run tessellation
    // // Returns array of indices that refer to the vertices of the input polygon.
    // // e.g: the index 6 would refer to {25, 75} in this example.
    // // Three subsequent indices form a triangle. Output triangles are clockwise.
    // std::vector<N> indices = mapbox::earcut<N>(polygon);

    m_fillVertices.clear(), m_outlineVertices.clear();
    m_fillVertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    m_outlineVertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    for (const size_t idx : fillTriangles)
        m_fillVertices.append(sf::Vertex(sf::Vector2f(fillVertices[idx].x, fillVertices[idx].y)));
    for (const size_t idx : outlineTriangles)
        m_outlineVertices.append(sf::Vertex(sf::Vector2f(outlineVertices[idx].x, outlineVertices[idx].y)));
}
// Thanks for StArray's code
// ReSharper disable once CppDFAConstantParameter
static void createMidSpinMesh(float width, sf::Angle a1, uint32_t m_interpolationLevel, sf::VertexArray& m_fillVertices,
                              sf::VertexArray& m_outlineVertices)
{
    a1 = a1.wrapUnsigned();
    float length = width;
    const float m1 = std::cos(a1.asRadians()), m2 = std::sin(a1.asRadians());

    // region outline
    std::vector<sf::Vector3f> fillVertices, outlineVertices;
    std::vector<size_t> fillTriangles, outlineTriangles;
    const sf::Vector3f midpoint{-m1 * 0.04f, -m2 * 0.04f, 0};
    width += outline;
    length += outline;
    {
        constexpr size_t count = 0;
        outlineVertices.push_back(midpoint + sf::Vector3f(length * m1 + width * m2, length * m2 - width * m1, 0));
        outlineVertices.push_back(midpoint + sf::Vector3f(length * m1 - width * m2, length * m2 + width * m1, 0));
        outlineVertices.push_back(midpoint + sf::Vector3f(-width * m2, width * m1, 0));
        outlineVertices.push_back(midpoint + sf::Vector3f(width * m2, -width * m1, 0));
        outlineVertices.push_back(midpoint + sf::Vector3f(-width * m1, -width * m2, 0));
        outlineVertices.push_back(midpoint + sf::Vector3f(width * m2, -width * m1, 0));
        outlineVertices.push_back(midpoint + sf::Vector3f(-width * m2, width * m1, 0));
        outlineTriangles.push_back(count);
        outlineTriangles.push_back(count + 1);
        outlineTriangles.push_back(count + 2);
        outlineTriangles.push_back(count + 2);
        outlineTriangles.push_back(count + 3);
        outlineTriangles.push_back(count);
        outlineTriangles.push_back(count + 4);
        outlineTriangles.push_back(count + 5);
        outlineTriangles.push_back(count + 6);
    }
    // endregion
    // region fill
    width -= outline * 2;
    length -= outline * 2;
    {
        const size_t count = fillVertices.size();
        fillVertices.push_back(midpoint + sf::Vector3f(length * m1 + width * m2, length * m2 - width * m1, 0));
        fillVertices.push_back(midpoint + sf::Vector3f(length * m1 - width * m2, length * m2 + width * m1, 0));
        fillVertices.push_back(midpoint + sf::Vector3f(-width * m2, width * m1, 0));
        fillVertices.push_back(midpoint + sf::Vector3f(width * m2, -width * m1, 0));
        fillVertices.push_back(midpoint + sf::Vector3f(-width * m1, -width * m2, 0));
        fillVertices.push_back(midpoint + sf::Vector3f(width * m2, -width * m1, 0));
        fillVertices.push_back(midpoint + sf::Vector3f(-width * m2, width * m1, 0));
        fillTriangles.push_back(count);
        fillTriangles.push_back(count + 1);
        fillTriangles.push_back(count + 2);
        fillTriangles.push_back(count + 2);
        fillTriangles.push_back(count + 3);
        fillTriangles.push_back(count);
        fillTriangles.push_back(count + 4);
        fillTriangles.push_back(count + 5);
        fillTriangles.push_back(count + 6);
    }
    // endregion

    m_fillVertices.clear(), m_outlineVertices.clear();
    m_fillVertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    m_outlineVertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    for (const size_t idx : fillTriangles)
        m_fillVertices.append(sf::Vertex(sf::Vector2f(fillVertices[idx].x, fillVertices[idx].y)));
    for (const size_t idx : outlineTriangles)
        m_outlineVertices.append(sf::Vertex(sf::Vector2f(outlineVertices[idx].x, outlineVertices[idx].y)));
}

TileShape::TileShape(const double l_lastAngle, const double l_angle, const double l_nextAngle)
{
    m_lastAngle = l_lastAngle, m_angle = l_angle, m_nextAngle = l_nextAngle;
}
void TileShape::update()
{
    constexpr float width = 0.275f, length = 0.5f;
    const auto angle = static_cast<float>(m_angle), nextAngle = static_cast<float>(m_nextAngle),
               lastAngle = static_cast<float>(m_lastAngle);
    if (m_nextAngle == 999)
        createMidSpinMesh(width, sf::degrees(angle + 180), m_interpolationLevel, m_fillVertices, m_outlineVertices);
    else
    {
        const sf::Angle startAngle = sf::degrees(m_angle == 999 ? lastAngle : angle + 180).wrapUnsigned(),
                        endAngle = sf::degrees(nextAngle).wrapUnsigned();
        createTileMesh(width, length, startAngle, endAngle, m_interpolationLevel, m_fillVertices, m_outlineVertices);
    }
    m_bounds = m_outlineVertices.getBounds();
}
sf::FloatRect TileShape::getLocalBounds() const { return m_bounds; }
sf::FloatRect TileShape::getGlobalBounds() const { return getTransform().transformRect(getLocalBounds()); }
bool TileShape::isPointInside(const sf::Vector2f point) const
{
    for (size_t i = 0; i < m_fillVertices.getVertexCount(); i += 3)
        if (pointIsInsideTriangle(
                {m_fillVertices[i].position, m_fillVertices[i + 1].position, m_fillVertices[i + 2].position}, point))
            return true;
    for (size_t i = 0; i < m_outlineVertices.getVertexCount(); i += 3)
        if (pointIsInsideTriangle(
                {m_outlineVertices[i].position, m_outlineVertices[i + 1].position, m_outlineVertices[i + 2].position},
                point))
            return true;
    return false;
}
void TileShape::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    states.texture = nullptr;
    target.draw(m_outlineVertices, states);
    target.draw(m_fillVertices, states);
}
sf::Color TileShape::getFillColor() const { return m_fillColor; }
void TileShape::setFillColor(const sf::Color color)
{
    m_fillColor = color;
    for (size_t i = 0; i < m_fillVertices.getVertexCount(); i++)
        m_fillVertices[i].color = m_fillColor;
}
sf::Color TileShape::getOutlineColor() const { return m_outlineColor; }
void TileShape::setOutlineColor(const sf::Color color)
{
    m_outlineColor = color;
    for (size_t i = 0; i < m_outlineVertices.getVertexCount(); i++)
        m_outlineVertices[i].color = m_outlineColor;
}
TileSprite::TileSprite(const double lastAngleDeg, const double angleDeg, const double nextAngleDeg)
{
    m_needToUpdate = true;
    m_twirl = false;
    m_active = false;
    m_trackColor = sf::Color::Black;
    m_trackStyle = AdoCpp::TrackStyle::Standard;
    m_opacity = 100;
    m_shape = TileShape(lastAngleDeg, angleDeg, nextAngleDeg);
    m_angleDeg = angleDeg;
    m_nextAngleDeg = nextAngleDeg;
    m_shape.update();
    {
        m_twirlShape.setOrigin({0.1f, 0.1f});
        m_twirlShape.setRadius(0.1f);
        m_twirlShape.setOutlineThickness(0.05f);
        m_twirlShape.setFillColor(sf::Color::Transparent);
    }
    {
        m_speedShape.setOrigin({0.15f, 0.15f});
        m_speedShape.setRadius(0.15f);
        m_speed = 0;
    }
}
void TileSprite::update()
{
    if (!m_needToUpdate)
        return;
    m_needToUpdate = false;
    static sf::Color m_color, m_borderColor;
    switch (m_trackStyle)
    {
    case AdoCpp::TrackStyle::Standard:
        m_color = m_trackColor, m_borderColor = m_color * sf::Color(127, 127, 127, 255);
        break;
    case AdoCpp::TrackStyle::Neon:
        m_color = sf::Color::Black, m_borderColor = m_trackColor;
        break;
    case AdoCpp::TrackStyle::NeonLight:
        m_borderColor = m_trackColor, m_color = m_borderColor * sf::Color(127, 127, 127, 255);
        break;
    case AdoCpp::TrackStyle::Basic:
    case AdoCpp::TrackStyle::Minimal:
    case AdoCpp::TrackStyle::Gems:
        m_color = m_trackColor, m_borderColor = sf::Color::Transparent;
        break;
    }
    m_color.a = static_cast<std::uint8_t>(static_cast<float>(m_color.a) * m_opacity / 100),
    m_borderColor.a = static_cast<std::uint8_t>(static_cast<float>(m_borderColor.a) * m_opacity / 100);
    if (m_active)
        m_color.r = m_color.r / 2, m_color.g = m_color.g / 2 + 255 / 2, m_color.b = m_color.b / 2,
        m_borderColor.r = m_borderColor.r / 2, m_borderColor.g = m_borderColor.g / 2 + 255 / 2,
        m_borderColor.b = m_borderColor.b / 2;
    // if (m_shape.getFillColor() != m_color)
    m_shape.setFillColor(m_color);
    // if (m_shape.getOutlineColor() != m_borderColor)
    m_shape.setOutlineColor(m_borderColor);
    const auto alpha = static_cast<std::uint8_t>(m_opacity / 100 * 255);
    if (m_twirl)
        m_twirlShape.setOutlineColor((m_twirl == 1 ? sf::Color(255, 0, 127) : sf::Color(127, 0, 255)) *
                                     sf::Color(255, 255, 255, alpha));
    if (m_speed)
        m_speedShape.setFillColor((m_speed == 1 ? sf::Color::Red : sf::Color::Blue) * sf::Color(255, 255, 255, alpha));
}
void TileSprite::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if (m_opacity == 0)
        return;
    states.transform *= getTransform();
    states.texture = nullptr;
    target.draw(m_shape, states);
    if (m_twirl)
        target.draw(m_twirlShape, states);
    if (m_speed)
        target.draw(m_speedShape, states);
}
// ReSharper disable once CppMemberFunctionMayBeConst
void TileSystem::parse()
{
    double lastAngle, nextAngle;
    m_tileSprites.clear();
    const auto& tiles = m_level.tiles;
    const auto& settings = m_level.settings;
    for (size_t i = 0; i < tiles.size(); i++)
    {
        const double angle = tiles[i].angle.deg();

        if (i == tiles.size() - 1)
            nextAngle = angle;
        else
            nextAngle = tiles[i + 1].angle.deg();

        if (i == 0)
            lastAngle = 0;
        else
            lastAngle = tiles[i - 1].angle.deg();

        m_tileSprites.emplace_back(lastAngle, angle, nextAngle);
    }
    double oBpm = settings.bpm, bpm = oBpm;
    for (const auto& tile : tiles)
    {
        for (const auto& event : tile.events)
        {
            if (const auto twirl = std::dynamic_pointer_cast<AdoCpp::Event::GamePlay::Twirl>(event))
            {
                m_tileSprites[twirl->floor].setTwirl(m_level.getAngle(twirl->floor + 1).deg() < 180 ? 1 : 2);
            }
            else if (const auto setSpeed = std::dynamic_pointer_cast<AdoCpp::Event::GamePlay::SetSpeed>(event))
            {
                if (setSpeed->speedType == AdoCpp::Event::GamePlay::SetSpeed::SpeedType::Bpm)
                    bpm = setSpeed->beatsPerMinute;
                else
                    bpm *= setSpeed->bpmMultiplier;
                if (bpm != oBpm)
                    m_tileSprites[setSpeed->floor].setSpeed(bpm > oBpm ? 1 : 2);
                oBpm = bpm;
            }
        }
    }
}
// ReSharper disable once CppMemberFunctionMayBeConst
void TileSystem::update()
{
    auto& tiles = m_level.tiles;
    for (size_t i = 0; i < m_tileSprites.size(); i++)
    {
        // ReSharper disable CppCStyleCast
        auto& sprite = m_tileSprites[i];
        const auto& tile = tiles[i];

        sprite.setPosition({(float)tile.pos.c.x, (float)tile.pos.c.y});
        sprite.setActive(m_activeTileIndex ? m_activeTileIndex == i : false);
        // sprite.setTrackColor(sf::Color(tile.color.toInteger()));
        // sprite.setTrackStyle(tile.trackStyle.c);
        sprite.setScale({(float)tile.scale.c.x / 100, (float)tile.scale.c.y / 100});
        sprite.setRotation(sf::degrees((float)tile.rotation.c));
        // sprite.setOpacity((float)tile.opacity);
        sprite.update();
        // ReSharper restore CppCStyleCast
    }
}
void TileSystem::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.texture = nullptr;
    const sf::Vector2f viewCenter(target.getView().getCenter());
    sf::Vector2f viewSize(target.getView().getSize());
    viewSize.x = viewSize.y = (std::max)(viewSize.x, viewSize.y) * 1.5f;
    const sf::FloatRect currentViewRect(viewCenter - viewSize / 2.f, viewSize);
    bool reachZero = false;
    for (size_t i = m_tileSprites.size() - 1;; i--)
    {
        if (reachZero)
            break;
        if (i == 0)
            reachZero = true;
        auto& sprite = m_tileSprites[i];
        const auto& tile = m_level.tiles[i];

        if (currentViewRect.findIntersection(sprite.getGlobalBoundsFaster()) && tile.scale.c.x != 0 &&
            tile.scale.c.y != 0)
        {
            sprite.setTrackColor(sf::Color(tile.color.toInteger()));
            sprite.setTrackStyle(tile.trackStyle.c);
            sprite.setOpacity(static_cast<float>(tile.opacity));
            sprite.update();
            target.draw(m_tileSprites[i]);
        }
    }
    // ReSharper disable once CppDFAConstantConditions
    if (m_activeTileIndex && m_tilePlaceMode)
    {
        // ReSharper disable once CppDFAUnreachableCode
        const auto& selectedTile = m_tileSprites[*m_activeTileIndex];
        const std::map<const char*, float> keyMap = {{"D", 0.f},   {"E", 45.f},  {"W", 90.f},  {"Q", 135.f},
                                                     {"A", 180.f}, {"Z", 225.f}, {"X", 270.f}, {"C", 315.f}};
        const std::map<const char*, float> shiftKeyMap = {{"J", 30.f},  {"Y", 60.f},  {"T", 120.f}, {"H", 150.f},
                                                          {"N", 210.f}, {"V", 240.f}, {"B", 300.f}, {"M", 330.f}};
        const std::map<const char*, float> shiftGraveKeyMap = {{"J", 15.f},  {"Y", 75.f},  {"T", 105.f}, {"H", 165.f},
                                                               {"N", 195.f}, {"V", 255.f}, {"B", 285.f}, {"M", 345.f}};
        const sf::Vector2f tilePos = selectedTile.getPosition();
        for (const auto& [key, value] : (m_tilePlaceMode == 1       ? keyMap
                                             : m_tilePlaceMode == 2 ? shiftKeyMap
                                                                    : shiftGraveKeyMap))
        {
            const sf::Vector2f rectPos = tilePos + sf::Vector2f(1.f, 0.f).rotatedBy(sf::degrees(value));
            sf::Text text{font, key, 128};
            text.setPosition(rectPos);
            text.setOrigin(text.getLocalBounds().getCenter());
            text.setScale(sf::Vector2f(0.002f, -0.002f));
            text.setFillColor(sf::Color::Black);

            sf::RectangleShape rect{{0.5f, 0.5f}};
            rect.setPosition(rectPos - sf::Vector2f(0.25f, 0.25f));
            rect.setFillColor(sf::Color::White);
            target.draw(rect);
            target.draw(text);
        }
    }
}
