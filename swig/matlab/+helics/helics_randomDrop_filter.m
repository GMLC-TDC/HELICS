function v = helics_randomDrop_filter()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 29);
  end
  v = vInitialized;
end
