function v = helics_terminated()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176308);
  end
  v = vInitialized;
end
