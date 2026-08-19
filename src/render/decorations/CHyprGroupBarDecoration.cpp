#include "CHyprGroupBarDecoration.hpp"
#include "../../Compositor.hpp"
#include "../Renderer.hpp"
#include "../../render/gl/GLFramebuffer.hpp"

#include <hyprtoolkit/core/EmbeddedBackend.hpp>
#include <hyprtoolkit/element/Rectangle.hpp>
#include <hyprtoolkit/core/Timer.hpp>

using namespace Render;
using namespace Hyprtoolkit;
using namespace Hyprutils::Memory;

class CMinimalHTLoop : public IEventLoop {
  public:
    void                                                         addFd(int fd, std::function<void()>&& cb) override {}
    void                                                         removeFd(int fd) override {}

    Hyprutils::Memory::CAtomicSharedPointer<Hyprtoolkit::CTimer> addTimer(const Hyprtoolkit::TimerDuration&                                                        timeout,
                                                                          std::function<void(Hyprutils::Memory::CAtomicSharedPointer<Hyprtoolkit::CTimer>, void*)> cb, void* data,
                                                                          bool force) override {
        return nullptr;
    }

    void addIdle(const std::function<void()>& cb) override {
        cb();
    }
    void cancelPending() override {}
    void enterLoop() override {}
};

static CSharedPointer<IEmbeddedBackend> g_pHTBackend;

CHyprGroupBarDecoration::CHyprGroupBarDecoration(PHLWINDOW pWindow) : IHyprWindowDecoration(pWindow), m_window(pWindow) {
    if (!g_pHTBackend) {
        IEmbeddedBackend::SCreationData data;
        data.eventLoop = makeShared<CMinimalHTLoop>();
        g_pHTBackend   = IEmbeddedBackend::create(data);
    }

    m_pHTSurface = g_pHTBackend->createSurface();

    auto rect = CRectangleBuilder::begin()
                    ->color([] { return Hyprtoolkit::CHyprColor{1.0f, 0.0f, 0.0f, 1.0f}; }) // red for now
                    ->size({CDynamicSize::HT_SIZE_PERCENT, CDynamicSize::HT_SIZE_PERCENT, {1.f, 1.f}})
                    ->commence();

    m_pHTSurface->rootElement()->addChild(rect);
}

CHyprGroupBarDecoration::~CHyprGroupBarDecoration() {
    ;
}

void CHyprGroupBarDecoration::draw(PHLMONITOR pMonitor, float const& a) {
    const auto ASSIGNEDBOX = assignedBoxGlobal();
    if (ASSIGNEDBOX.empty())
        return;

    Vector2D pixelSize = ASSIGNEDBOX.size() * pMonitor->scale();

    if (!m_pFBO) {
        m_pFBO = makeUnique<Render::GL::CGLFramebuffer>("HT_GroupBarFBO");
    }

    if (m_pFBO->m_size != pixelSize) {
        m_pFBO->alloc(pixelSize.x, pixelSize.y, DRM_FORMAT_ARGB8888);
        m_pHTSurface->resize(ASSIGNEDBOX.size(), pixelSize, pMonitor->scale());
    }

    GLint oldFB;
    GLint oldViewport[4];
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldFB);
    glGetIntegerv(GL_VIEWPORT, oldViewport);

    m_pFBO->bind();
    glViewport(0, 0, pixelSize.x, pixelSize.y);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_pHTSurface->render(0);

    glBindFramebuffer(GL_FRAMEBUFFER, oldFB);
    glViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);

    CTexPassElement::SRenderData data;
    data.tex = m_pFBO->getTexture();
    data.box = ASSIGNEDBOX;
    data.a   = a;

    g_pHyprRenderer->addPassElement(makeUnique<CTexPassElement>(data));
}

void CHyprGroupBarDecoration::updateWindow(PHLWINDOW) {
    damageEntire();
}

void CHyprGroupBarDecoration::damageEntire() {
    auto box = assignedBoxGlobal();
    box.translate(m_window->m_floatingOffset);
    g_pHyprRenderer->damageBox(box);
}

SDecorationPositioningInfo CHyprGroupBarDecoration::getPositioningInfo() {
    SDecorationPositioningInfo info;
    info.policy         = DECORATION_POSITION_STICKY;
    info.edges          = DECORATION_EDGE_TOP;
    info.priority       = 100;
    info.reserved       = true;
    info.desiredExtents = {.topLeft = {0, 20}, .bottomRight = {0, 0}};
    return info;
}

void CHyprGroupBarDecoration::onPositioningReply(const SDecorationPositioningReply& reply) {
    m_assignedBox = reply.assignedGeometry;
}

eDecorationType CHyprGroupBarDecoration::getDecorationType() {
    return DECORATION_GROUPBAR;
}

eDecorationLayer CHyprGroupBarDecoration::getDecorationLayer() {
    return DECORATION_LAYER_OVER;
}

uint64_t CHyprGroupBarDecoration::getDecorationFlags() {
    return 0;
}

std::string CHyprGroupBarDecoration::getDisplayName() {
    return "GroupBar";
}

CBox CHyprGroupBarDecoration::assignedBoxGlobal() {
    CBox box = m_assignedBox;
    box.translate(g_pDecorationPositioner->getEdgeDefinedPoint(DECORATION_EDGE_TOP, m_window.lock()));
    if (m_window->m_workspace && !m_window->m_pinned)
        box.translate(m_window->m_workspace->m_renderOffset->value());
    return box.round();
}

void refreshGroupBarGradients() {
    ;
}
