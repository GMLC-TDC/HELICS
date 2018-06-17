function v = helics_discard()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176307);
  end
  v = vInitialized;
end
