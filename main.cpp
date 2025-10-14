
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// This file is part of the frequency-analyzer application.
// It is licensed to you under the terms of the MIT license.
// http://opensource.org/licenses/MIT
//
// Copyright (c) 2014 Timur Kristóf

#include <QApplication>
#include <QQmlApplicationEngine>
#include "waterfallitem.h"
#include <QQmlContext>

int main(int argc, char *argv[])
{
    // 🟢 Active la lecture des fichiers locaux via XMLHttpRequest
    qputenv("QSG_RHI_BACKEND", "opengl");  // 🔥 force le backend OpenGL
    qputenv("QML_XHR_ALLOW_FILE_READ", "1");

    QApplication app(argc, argv);
    WaterfallItem waterfallItem;
    qmlRegisterType<WaterfallItem>("hu.timur", 1, 0, "Waterfall");

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("waterfallItem",&waterfallItem);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
