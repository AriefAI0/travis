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

    QVERIFY_NO_THROW((void)appContext.projectService());
    QVERIFY_NO_THROW((void)appContext.sessionService());
    QVERIFY_NO_THROW((void)appContext.structureService());
    QVERIFY_NO_THROW((void)appContext.executionService());
    QVERIFY_NO_THROW((void)appContext.resultService());
    QVERIFY_NO_THROW((void)appContext.resultMediaService());
    QVERIFY_NO_THROW((void)appContext.videoService());
    QVERIFY_NO_THROW((void)appContext.inspectionClipService());
}

} // namespace

QTEST_APPLESS_MAIN(AppContextTest)

#include "app_context_test.moc"
