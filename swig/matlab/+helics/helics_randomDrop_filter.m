function v = helics_randomDrop_filter()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1432107616);
  end
  v = vInitialized;
end
