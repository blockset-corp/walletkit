package com.blockset.walletkit.demo.walletconnect.msg;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;

import static com.google.common.base.Preconditions.checkNotNull;

public class PeerMeta {

    @JsonCreator
    public static PeerMeta create(@JsonProperty("description") String      description,
                                  @JsonProperty("url")         String      url,
                                  @JsonProperty("icons")       String[]    icons,
                                  @JsonProperty("name")        String      name    ) {
        return new PeerMeta(checkNotNull(description),
                            checkNotNull(url),
                            checkNotNull(icons),
                            checkNotNull(name));
    }

    private final String    description;
    private final String    url;
    private final String[]  icons;
    private final String    name;

    private PeerMeta(String     description,
                     String     url,
                     String[]   icons,
                     String     name    ) {
        this.description = description;
        this.url = url;
        this.icons = icons;
        this.name = name;
    }

    // getters

    @JsonProperty("description")
    public String getDescription() { return description; }

    @JsonProperty("url")
    public String getUrl() { return url; }

    @JsonProperty("icons")
    public String[] getIcons() { return icons; }

    @JsonProperty("name")
    public String getName() { return name; }
}
