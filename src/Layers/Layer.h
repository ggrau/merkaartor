#ifndef LAYER_H_
#define LAYER_H_

#include "MapTypedef.h"
#include "Coord.h"
#include "Feature.h"

#include <QProgressDialog>

#include "ILayer.h"

class QString;
class QprogressDialog;

class Feature;
class LayerPrivate;
class MapAdapter;
class Layer;
class LayerManager;
class LayerWidget;
class WMSMapAdapter;
class TileMapAdapter;
class TrackSegment;
class IMapAdapter;
class Document;

struct IndexFindContext;

class Layer : public QObject, public ILayer
{
    Q_OBJECT

public:
    Layer();
    Layer(const QString& aName);

private:
    Layer(const Layer& aLayer);

public:
    typedef enum {
        UndefinedType,
        DeletedLayerType,
        DirtyLayerType,
        DrawingLayerType,
        ExtractedLayerType,
        ImageLayerType,
        TrackLayerType,
        UploadedLayerType,
        FilterLayerType,
        OsmBugsLayer,
        MapDustLayer
    } LayerType;

    enum LayerGroup {
        None				= 0x00000000,
        Map				    = 0x00000001,
        Draw				= 0x00000002,
        Tracks				= 0x00000004,
        Filters				= 0x00000008,
        Special				= 0x00000010,
        All					= 0x0000ffff
    };

    Q_DECLARE_FLAGS(LayerGroups, LayerGroup)

public:
    virtual ~Layer();

    void setName(const QString& aName);
    const QString& name() const;
    void setDescription(const QString& aDesc);
    const QString& description() const;
    bool isVisible() const;
    bool isSelected() const;
    bool isEnabled() const;

    virtual void add(Feature* aFeature);
    virtual void remove(Feature* aFeature);
    virtual void deleteFeature(Feature* aFeature);
    virtual void clear();
    virtual void deleteAll();
    bool exists(Feature* aFeature) const;
    int getDisplaySize() const;
    virtual int size() const;
    int get(Feature* aFeature);
    QList<Feature *> get();
    Feature* get(int i);
    const Feature* get(int i) const;
    virtual Feature* get(const IFeature::FId& id);
    void notifyIdUpdate(const IFeature::FId& id, Feature* aFeature);

    virtual void setDocument(Document* aDocument);
    Document* getDocument();

    LayerWidget* getWidget(void);
    void deleteWidget(void);
    virtual void updateWidget() {}

    virtual void setVisible(bool b);
    virtual void setSelected(bool b);
    virtual void setEnabled(bool b);
    virtual void setReadonly(bool b);
    virtual void setUploadable(bool b);
    virtual LayerWidget* newWidget(void) = 0;

    virtual void setAlpha(const qreal alpha);
    virtual qreal getAlpha() const;

    void setId(const QString& id);
    const QString& id() const;

    virtual QString toMainHtml();
    virtual QString toHtml();
    virtual QString toPropertiesHtml();

    virtual bool toXML(QXmlStreamWriter& stream, bool asTemplate, QProgressDialog * progress);
    static Layer* fromXML(Layer* l, Document* d, QXmlStreamReader& stream, QProgressDialog * progress);

    virtual CoordBox boundingBox();

    virtual /* const */ LayerType classType() const = 0;
    virtual const LayerGroups classGroups() const = 0;

    int incDirtyLevel(int inc=1);
    int decDirtyLevel(int inc=1);
    int getDirtyLevel() const;
    int setDirtyLevel(int newLevel);
    int getDirtySize() const;

    virtual bool canDelete() const;
    virtual bool isUploadable() const;
    virtual bool isReadonly() const;
    virtual bool isTrack() const {return false;}

protected:
    LayerPrivate* p;
    LayerWidget* theWidget;
    mutable QString Id;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Layer::LayerGroups)

class DrawingLayer : public Layer
{
    Q_OBJECT

public:
    DrawingLayer();
    DrawingLayer(const QString& aName);
    virtual ~DrawingLayer();

    virtual LayerWidget* newWidget(void);

    virtual bool toXML(QXmlStreamWriter& stream, bool asTemplate, QProgressDialog * progress);
    static DrawingLayer* fromXML(Document* d, QXmlStreamReader& stream, QProgressDialog * progress);
    static DrawingLayer* doFromXML(DrawingLayer* l, Document* d, QXmlStreamReader& stream, QProgressDialog * progress);

    virtual /* const */ LayerType classType() const {return Layer::DrawingLayerType;}
    virtual const LayerGroups classGroups() const {return (Layer::Draw);}
};

class TrackLayer : public Layer
{
    Q_OBJECT
public:
    TrackLayer(const QString& aName="", const QString& filaname="");
    virtual ~TrackLayer();

    virtual LayerWidget* newWidget(void);

    virtual void extractLayer();
    virtual const QString getFilename();

    virtual QString toHtml();
    virtual bool toXML(QXmlStreamWriter& stream, bool asTemplate, QProgressDialog * progress);
    static TrackLayer* fromXML(Document* d, QXmlStreamReader& stream, QProgressDialog * progress);

    virtual /* const */ LayerType classType() const {return Layer::TrackLayerType;}
    virtual const LayerGroups classGroups() const {return(Layer::Tracks);}

    virtual bool isUploadable() const {return true;}
    virtual bool isTrack() const {return true;}

protected:
    QString Filename;
};

class SpecialLayer : public TrackLayer
{
    Q_OBJECT
public:
    SpecialLayer(const QString& aName="", LayerType type=Layer::UndefinedType, const QString& filename="");

    virtual LayerWidget* newWidget(void);

    virtual void refreshLayer();

    virtual /* const */ LayerType classType() const {return m_type;}
    virtual const LayerGroups classGroups() const {return(Layer::Special);}

    virtual bool isUploadable() const {return false;}
    virtual bool isTrack() const {return true;}

protected:
    LayerType m_type;
};

class DirtyLayer : public DrawingLayer
{
    Q_OBJECT
public:
    DirtyLayer(const QString& aName);
    virtual ~DirtyLayer();

    static DirtyLayer* fromXML(Document* d, QXmlStreamReader& stream, QProgressDialog * progress);

    virtual /* const */ LayerType classType() const {return Layer::DirtyLayerType;}
    virtual const LayerGroups classGroups() const {return(Layer::Map|Layer::Draw);}

    virtual LayerWidget* newWidget(void);

    virtual bool canDelete() const { return false; }

};

class UploadedLayer : public DrawingLayer
{
    Q_OBJECT
public:
    UploadedLayer(const QString& aName);
    virtual ~UploadedLayer();

    static UploadedLayer* fromXML(Document* d, QXmlStreamReader& stream, QProgressDialog * progress);

    virtual /* const */ LayerType classType() const {return Layer::UploadedLayerType;}
    virtual const LayerGroups classGroups() const {return(Layer::Map|Layer::Draw);}

    virtual LayerWidget* newWidget(void);

    virtual bool canDelete() const { return false; }
};

class DeletedLayer : public DrawingLayer
{
    Q_OBJECT
public:
    DeletedLayer(const QString& aName);
    virtual ~DeletedLayer();

    virtual bool toXML(QXmlStreamWriter& stream, bool asTemplate, QProgressDialog * progress);
    static DeletedLayer* fromXML(Document* d, QXmlStreamReader& stream, QProgressDialog * progress);

    virtual /* const */ LayerType classType() const {return Layer::DeletedLayerType;}
    virtual const LayerGroups classGroups() const {return(Layer::None);}
    virtual LayerWidget* newWidget(void);

    virtual bool isUploadable() const {return false;}
    virtual bool canDelete() const { return false; }
};

class FilterLayer : public Layer
{
    Q_OBJECT
public:
    FilterLayer(const QString& aId, const QString& aName, const QString& aFilter);
    virtual ~FilterLayer();

    bool toXML(QXmlStreamWriter& stream, bool asTemplate, QProgressDialog * progress);
    static FilterLayer* fromXML(Document* d, QXmlStreamReader& stream, QProgressDialog * progress);

    virtual /* const */ LayerType classType() const {return Layer::FilterLayerType;}
    virtual const LayerGroups classGroups() const {return(Layer::Filters);}

    virtual LayerWidget* newWidget(void);

    virtual bool canDelete() const { return true; }

public:
    virtual void setFilter(const QString& aFilter);
    virtual QString filter() { return theSelectorString; }
    virtual TagSelector* selector() { return theSelector; }

protected:
    QString theSelectorString;
    TagSelector* theSelector;

};


Q_DECLARE_METATYPE ( QUuid )

#endif


