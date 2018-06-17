function v = helics_randomDrop_filter()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176332);
  end
  v = vInitialized;
end
