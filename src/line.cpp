#include "header.h"

void QeLine::initialize(QeAssetXML *_property, QeObject *_owner) {
    QeComponent::initialize(_property, _owner);

    targetTransformOID = initProperty->getXMLValuei("targetTransformOID");
    color = initProperty->getXMLValueRGB("color");

    VK->createBuffer(modelBuffer, sizeof(bufferData), nullptr);

    modelData = AST->getModel("line", false, (float *)&color);

    shaderKey = "line";
    AST->setGraphicsShader(graphicsShader, nullptr, shaderKey);

    graphicsPipeline.bAlpha = false;
    GRAP->models.push_back(this);

    VK->createDescriptorSet(descriptorSet);
    VK->updateDescriptorSet(&createDescriptorSetModel(), descriptorSet);
}

void QeLine::update1() {
    if (targetTransformOID == 0) return;

    QeTransform *targetTransform = (QeTransform *)SCENE->findComponent(eComponent_transform, targetTransformOID);
    if (targetTransform == 0) return;

    QeVector3f targetPos = targetTransform->worldPosition();
    QeVector3f pos = owner->transform->worldPosition();

    QeVector3f vec = targetPos - pos;
    float size = MATH->fastSqrt(MATH->length(vec));
    QeVector3f scale = {size, size, size};

    // LOG("line x: " + vec.x + "  y: " + vec.y + "  z: " + vec.z);
    bufferData.model = MATH->getTransformMatrix(owner->transform->worldPosition(), MATH->vectorToEulerAngles(vec), scale);

    VK->setMemoryBuffer(modelBuffer, sizeof(bufferData), &bufferData);
}

/*QeDataDescriptorSetModel QeLine::createDescriptorSetModel() {
        QeDataDescriptorSetModel descriptorSetData;
        descriptorSetData.modelBuffer = modelBuffer.buffer;
        return descriptorSetData;
}*/
