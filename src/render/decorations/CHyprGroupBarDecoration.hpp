#pragma once
#include "IHyprWindowDecoration.hpp"
#include "../Framebuffer.hpp"
#include <hyprtoolkit/window/EmbeddedSurface.hpp>

class CHyprGroupBarDecoration : public IHyprWindowDecoration {
  public:
    CHyprGroupBarDecoration(PHLWINDOW);
    virtual ~CHyprGroupBarDecoration();

    virtual SDecorationPositioningInfo getPositioningInfo();
    virtual void                       onPositioningReply(const SDecorationPositioningReply& reply);
    virtual void                       draw(PHLMONITOR, float const& a);
    virtual eDecorationType            getDecorationType();
    virtual void                       updateWindow(PHLWINDOW);
    virtual void                       damageEntire();
    virtual eDecorationLayer           getDecorationLayer();
    virtual uint64_t                   getDecorationFlags();
    virtual std::string                getDisplayName();

  private:
    CBox         m_assignedBox;
    PHLWINDOWREF m_window;

    Hyprutils::Memory::CSharedPointer<Hyprtoolkit::IEmbeddedSurface> m_pHTSurface;
    UP<Render::IFramebuffer>                                         m_pFBO;

    CBox                                                             assignedBoxGlobal();
};
