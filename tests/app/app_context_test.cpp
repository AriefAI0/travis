#include <QtTest>

#include "app/app_context.h"

namespace {

class AppContextTest : public QObject {
    Q_OBJECT

private slots:
    // Verifies app context can initialize the shared database and service graph.
    void initialize_buildsServiceGraph();
};

void AppContextTest::initialize_buildsServiceGraph() {
    travis::app::AppContext appContext;

    QVERIFY2(appContext.initialize(), qPrintable(appContext.lastError()));

    (void)appContext.projectService();
    (void)appContext.sessionService();
    (void)appContext.structureService();
    (void)appContext.executionService();
    (void)appContext.resultService();
    (void)appContext.resultMediaService();
    (void)appContext.videoService();
    (void)appContext.inspectionClipService();

    QVERIFY(true);
}

} // namespace

QTEST_GUILESS_MAIN(AppContextTest)

#include "app_context_test.moc"
